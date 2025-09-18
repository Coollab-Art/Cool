#pragma once
#include <condition_variable>
#include <reg/src/AnyId.hpp>
#include <shared_mutex>
#include <thread>
#include "Task.hpp"
#include "TaskCoroutine.hpp"
#include "WaitToExecuteTask.hpp"

namespace Cool {

class TaskManager {
public:
    TaskManager();
    ~TaskManager()                                 = default;
    TaskManager(TaskManager const&)                = delete;
    TaskManager& operator=(TaskManager const&)     = delete;
    TaskManager(TaskManager&&) noexcept            = delete;
    TaskManager& operator=(TaskManager&&) noexcept = delete;

    void submit(std::shared_ptr<Task> const& task);
    void submit(std::shared_ptr<WaitToExecuteTask> const& condition, std::shared_ptr<Task> const& task);

    void cancel_all(reg::AnyId const& owner_id);

    auto threads_count() const -> size_t { return _threads.size(); }

    void imgui_show_debug_tasks_list();

private:
    friend class AppManager;
    void update_on_main_thread();
    auto list_of_tasks_that_need_user_confirmation_before_killing() const -> std::string;
    void shut_down();

private:
    void thread_update_loop();
    void cancel_all();
    void cancel_if(std::function<bool(Task const&)> const& predicate);

    static void cancel_task_that_is_waiting(Task&);
    static void cancel_task_that_is_executing(Task&);

private:
    struct TaskAndCondition {
        std::shared_ptr<Task>              task;
        std::shared_ptr<WaitToExecuteTask> condition;
    };
    struct TaskAndCoroutine {
        std::shared_ptr<Task> task;
        TaskCoroutine         coroutine;
    };

    std::list<TaskAndCondition> _tasks_waiting{};
    mutable std::shared_mutex   _tasks_waiting_mutex{};

    std::deque<TaskAndCoroutine*> _tasks_to_process{}; // processing_queue
    std::list<TaskAndCoroutine>   _all_tasks_in_progress{};
    mutable std::shared_mutex     _tasks_to_process_mutex{}; // rename as tasks_in_progress_mutex

    std::vector<std::thread>    _threads{};
    std::condition_variable_any _wake_up_thread{};
    std::condition_variable_any _wait_for_threads_to_finish{};
    std::atomic<bool>           _is_shutting_down{false};
};

inline auto task_manager() -> TaskManager&
{
    static auto instance = TaskManager{};
    return instance;
}

} // namespace Cool