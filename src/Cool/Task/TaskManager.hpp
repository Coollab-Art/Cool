#pragma once
#include <condition_variable>
#include <reg/src/AnyId.hpp>
#include <shared_mutex>
#include <thread>
#include "Task.hpp"
#include "WaitToExecuteTask.hpp"

namespace Cool {

// TODO(Tasks) need to block till all tasks are finished before shutting down the app (eg test what happens when closing the window during a video export)
// and also, tell tasks to cancel asap (tasks decide what they do with that info, maybe they are doing something like saving an image and we want to wait till it completes)
// and also the tasks that have not started yet, they must tell us if they need to run or can be skipped
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

    auto num_tasks_waiting_for_thread(reg::AnyId const& owner_id) const -> size_t;
    auto num_tasks_waiting_for_condition(reg::AnyId const& owner_id) const -> size_t;
    auto num_tasks_processing(reg::AnyId const& owner_id) const -> size_t;
    auto threads_count() const -> size_t { return _threads.size(); }

private:
    friend class TestTasks;
    auto num_tasks_waiting_for_thread() const -> size_t;
    auto num_tasks_waiting_for_condition() const -> size_t;
    auto num_tasks_processing() const -> size_t;

private:
    friend class AppManager;
    void update_on_main_thread();
    auto list_of_tasks_that_need_user_confirmation_before_killing() const -> std::string;
    void shut_down();

private:
    void thread_update_loop();
    void cancel_all();
    void cancel_if(std::function<bool(Task const&)> const& predicate);
    void execute_task_asap(std::shared_ptr<Task> const& task);

    static void execute_task(Task&);
    static void cancel_task_that_is_waiting(Task&);
    static void cancel_task_that_is_executing(Task&);

private:
    std::deque<std::shared_ptr<Task>> _tasks_waiting{};
    std::list<std::shared_ptr<Task>>  _tasks_processing{};
    mutable std::shared_mutex         _tasks_mutex{};

    std::vector<std::thread>    _threads{};
    std::condition_variable_any _wake_up_thread{};
    std::condition_variable_any _wait_for_threads_to_finish{};
    std::atomic<bool>           _is_shutting_down{false};

    struct TaskAndCondition {
        std::shared_ptr<Task>              task;
        std::shared_ptr<WaitToExecuteTask> condition;
    };
    std::list<TaskAndCondition> _tasks_with_condition;
    mutable std::shared_mutex   _tasks_with_condition_mutex;
};

inline auto task_manager() -> TaskManager&
{
    static auto instance = TaskManager{};
    return instance;
}

} // namespace Cool