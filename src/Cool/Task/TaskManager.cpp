#include "TaskManager.hpp"
#include <imgui.h>
#include <algorithm>
#include <chrono>
#include <exception>
#include <memory>
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/Log/Log.hpp"
#include "Cool/Time/time_formatted_hms.h"
#include "TaskCoroutine.hpp"

namespace Cool {

TaskManager::TaskManager()
{
    auto const size = std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 1;
    _threads.reserve(size);
    for (size_t i = 0; i < size; ++i)
        _threads.emplace_back([&]() { thread_update_loop(); });
}

void TaskManager::shut_down()
{
    // Make sure we don't accept tasks anymore, tell threads to stop asap
    _is_shutting_down.store(true);
    // Remove waiting tasks, tell processing tasks to finish asap
    cancel_all();
    { // Wait for threads that are processing a task to finish
        auto lock = std::unique_lock{_tasks_to_process_mutex};
        _wait_for_threads_to_finish.wait(lock, [&] { return _all_tasks_in_progress.empty(); });
    }
    // Wake up all threads that were not processing a task and let them realize that _is_shutting_down == true
    _wake_up_thread.notify_all();
    // Close all threads
    for (std::thread& thread : _threads)
        thread.join();
}

void TaskManager::cancel_task_that_is_waiting(Task& task)
{
    // Task has not started execute(), so there is no need to call cancel()
    task.cleanup(true /*has_been_canceled*/);
}

void TaskManager::thread_update_loop()
{
    while (true)
    {
        TaskAndCoroutine* task = nullptr;
        { // Grab a task from the queue
            auto lock = std::unique_lock{_tasks_to_process_mutex};
            _wake_up_thread.wait(lock, [&] { return !_tasks_to_process.empty() || _is_shutting_down.load(); });
            if (_is_shutting_down.load())
                break;
            if (_tasks_to_process.empty())
            {
                assert(false); // Should not happen I think
                continue;
            }
            task = _tasks_to_process.front();
            _tasks_to_process.pop_front();
        }

        try
        {
#if DEBUG
            auto const start = std::chrono::steady_clock::now();
#endif
            task->coroutine.do_some_work();
#if DEBUG
            auto const end = std::chrono::steady_clock::now();
            if (end - start > 1s)
                Cool::Log::warning(task->task->name(), fmt::format("Took {} without pausing the coroutine. We should pause this coroutine more often", time_formatted_hms(end - start)));
#endif
        }
        catch (std::exception const& e)
        {
            Cool::Log::internal_error(task->task->name(), fmt::format("Threw an uncaught exception:\n{}", e.what()));
        }

        if (task->coroutine.has_finished() || task->task->has_been_canceled())
        {
            task->task->cleanup(task->task->has_been_canceled());
            auto lock = std::unique_lock{_tasks_to_process_mutex};
            std::erase_if(_all_tasks_in_progress, [&](auto const& bob) {
                return &bob == task;
            });
        }
        else
        {
            auto lock = std::unique_lock{_tasks_to_process_mutex};
            _tasks_to_process.push_back(task);
            _wake_up_thread.notify_one();
        }

        if (_is_shutting_down.load())
        {
            _wait_for_threads_to_finish.notify_one();
            break;
        }
    }
}

void TaskManager::update_on_main_thread()
{
    auto tasks_to_start  = std::vector<std::shared_ptr<Task>>{}; // We don't submit the tasks immediately in the loop, because they might be executed immediately, and if they want to submit other tasks this would conflict with the lock
    auto tasks_to_cancel = std::vector<std::shared_ptr<Task>>{}; // We don't cancel the tasks immediately in the loop, because the custom code in their cleanup() might conflict with the lock

    {
        auto lock = std::unique_lock{_tasks_waiting_mutex};
        for (auto it = _tasks_waiting.begin(); it != _tasks_waiting.end();)
        {
            if (!it->condition || it->condition->wants_to_execute())
            {
                tasks_to_start.emplace_back(it->task);
                it = _tasks_waiting.erase(it);
            }
            else if (it->condition->wants_to_cancel())
            {
                tasks_to_cancel.emplace_back(it->task);
                it = _tasks_waiting.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    {
        auto lock = std::unique_lock{_tasks_to_process_mutex};
        for (auto const& task : tasks_to_start)
        {
            _all_tasks_in_progress.emplace_back(TaskAndCoroutine{task, task->execute()});
            _tasks_to_process.push_back(&_all_tasks_in_progress.back());
            _wake_up_thread.notify_one();
        }
    }
    for (auto const& task : tasks_to_cancel)
        cancel_task_that_is_waiting(*task);
}

void TaskManager::submit(std::shared_ptr<Task> const& task)
{
    submit(nullptr /*condition*/, task);
}

void TaskManager::submit(std::shared_ptr<WaitToExecuteTask> const& condition, std::shared_ptr<Task> const& task)
{
    if (_is_shutting_down.load())
        return;

    task->on_submit();

    auto lock = std::unique_lock{_tasks_waiting_mutex};
    _tasks_waiting.emplace_back(TaskAndCondition{task, condition});
}

void TaskManager::cancel_if(std::function<bool(Task const&)> const& predicate)
{
    {
        auto lock = std::unique_lock{_tasks_waiting_mutex};

        for (TaskAndCondition& task : _tasks_waiting)
        {
            if (!predicate(*task.task))
                continue;

            cancel_task_that_is_waiting(*task.task);
            task.task = nullptr; // Mark them as nullptr so that we can erase_if afterwards without having to check the predicate again
        }
        std::erase_if(_tasks_waiting, [](TaskAndCondition const& task) { return task.task == nullptr; });
    }
    {
        auto lock = std::unique_lock{_tasks_to_process_mutex};

        for (TaskAndCoroutine& task : _all_tasks_in_progress)
        {
            if (!predicate(*task.task))
                continue;

            task.task->cancel();
            // task.task->_completion.store(Task::Completion::Canceled);

            auto const it = std::find_if(_tasks_to_process.begin(), _tasks_to_process.end(), [&](TaskAndCoroutine* bob) { return bob == &task; });
            if (it != _tasks_to_process.end())
            {
                task.task->cleanup(true /*has_been_cancelled*/);
                task.task = nullptr; // Mark them as nullptr so that we can erase_if afterwards without having to check the predicate again
                _tasks_to_process.erase(it);
            }
            // else
            // The thread that is running the task will clean it up once it finishes
        }
        std::erase_if(_all_tasks_in_progress, [](TaskAndCoroutine const& task) { return task.task == nullptr; });
    }
}

void TaskManager::cancel_all()
{
    cancel_if([&](Task const&) { return true; });
}

void TaskManager::cancel_all(reg::AnyId const& owner_id)
{
    cancel_if([&](Task const& task) { return task.owner_id() == owner_id; });
}

void TaskManager::imgui_show_debug_tasks_list()
{
    if (ImGui::Button("Cancel all tasks"))
        cancel_all();

    auto task_to_cancel = std::shared_ptr<Task>{nullptr};
    {
        auto lock  = std::shared_lock{_tasks_waiting_mutex};
        auto lock2 = std::shared_lock{_tasks_to_process_mutex};

        ImGui::SeparatorText(fmt::format("Tasks Waiting ({})", _tasks_waiting.size()).c_str());
        for (auto const& task : _tasks_waiting)
        {
            ImGui::PushID(&task);
            if (ImGui::Button("Cancel"))
                task_to_cancel = task.task;
            ImGui::SameLine();
            ImGui::TextUnformatted(fmt::format("  - {}", task.task->name()).c_str());
            ImGui::PopID();
        }

        ImGui::SeparatorText(fmt::format("Tasks in Progress ({})", _all_tasks_in_progress.size()).c_str());
        for (auto const& task : _all_tasks_in_progress)
        {
            ImGui::PushID(&task);
            if (ImGui::Button("Cancel"))
                task_to_cancel = task.task;
            ImGui::SameLine();
            ImGui::TextUnformatted(fmt::format("  - {}", task.task->name()).c_str());
            ImGui::PopID();
        }
    }
    if (task_to_cancel != nullptr)
        cancel_if([&](Task const& task) { return &task == task_to_cancel.get(); });
}

auto TaskManager::list_of_tasks_that_need_user_confirmation_before_killing() const -> std::string
{
    auto       list_of_tasks  = ""s;
    auto const maybe_add_task = [&](Task const& task) {
        if (!task.needs_user_confirmation_to_cancel_when_closing_app())
            return;
        if (!list_of_tasks.empty())
            list_of_tasks += '\n';
        list_of_tasks += fmt::format(" - {}", task.name());
    };

    {
        auto lock1 = std::shared_lock{_tasks_waiting_mutex};
        auto lock2 = std::shared_lock{_tasks_to_process_mutex};

        for (TaskAndCoroutine const& task : _all_tasks_in_progress)
            maybe_add_task(*task.task);
        for (TaskAndCondition const& task : _tasks_waiting)
            maybe_add_task(*task.task);
    }

    return list_of_tasks;
}

} // namespace Cool