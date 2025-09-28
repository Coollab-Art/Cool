#pragma once
#include <reg/src/AnyId.hpp>

namespace Cool {

class Task {
public:
    // Passing an owner_id allows to cancel all tasks with a given owner_id at once. This is optional
    explicit Task(reg::AnyId const& owner_id = {})
        : _owner_id{owner_id}
    {}
    virtual ~Task()                  = default;
    Task(Task const&)                = delete;
    Task& operator=(Task const&)     = delete;
    Task(Task&&) noexcept            = delete;
    Task& operator=(Task&&) noexcept = delete;

    auto owner_id() const -> reg::AnyId const& { return _owner_id; }
    auto has_been_executed() const -> bool { return _completion.load() == Completion::FinishedExecuting; }
    auto has_been_canceled() const -> bool { return _completion.load() == Completion::Canceled; }

    /// Name that will be displayed to the user, for example when they try to close the app while tasks are still in progress
    virtual auto name() const -> std::string = 0;

private:
    friend class TaskManager;

    /// Do the actual work of the task
    virtual void execute() = 0;

    /// Called as soon as the task is given to task_manager().submit()
    virtual void on_submit() {}
    /// Always called (if the task has been submitted to the TaskManager), either after execute() is finished, or after canceling the task. on_submit() is guaranteed to have been called before. This is called only once
    /// You shouldn't put your cleanup logic in the destructor, because the task might be kept alive by someone referencing it long after it has been canceled / has finished executing
    virtual void cleanup(bool /* has_been_canceled */) {}

    /// Allows us to do optimizations if we know that the task will run very quickly
    virtual auto is_quick_task() const -> bool = 0;

    /// Request to finish the task as quickly as possible, if execute() is currently running it should exit as soon as possible
    /// Don't put your cleanup logic in cancel(), put it cleanup() because cancel() will only run if do_work() is currently running
    /// Must be thread-safe because it can be called from various threads, possibly while execute() is running on another thread
    virtual void cancel() = 0;

    /// When the user tries to close the app, should we warn them that this task has not started and/or finished executing, and would be canceled?
    virtual auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool = 0;

private:
    reg::AnyId _owner_id{};

    enum class Completion {
        None,
        Canceled,
        FinishedExecuting,
    };
    std::atomic<Completion> _completion{Completion::None};
};

} // namespace Cool