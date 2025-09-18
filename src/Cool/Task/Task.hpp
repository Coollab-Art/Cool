#pragma once
#include <reg/src/AnyId.hpp>
#include "TaskCoroutine.hpp"

namespace Cool {

class Task {
public:
    // Passing an owner_id allows to cancel all tasks with a given owner_id at once. This is optional
    // `name` is the name that will be displayed to the user, for example when they try to close the app while tasks are still in progress.
    explicit Task(std::string name, reg::AnyId const& owner_id = {})
        : _owner_id{owner_id}
        , _name{std::move(name)}
    {}
    virtual ~Task();
    Task(Task const&)                = delete;
    Task& operator=(Task const&)     = delete;
    Task(Task&&) noexcept            = delete;
    Task& operator=(Task&&) noexcept = delete;

    auto owner_id() const -> reg::AnyId const& { return _owner_id; }
    auto has_been_executed() const -> bool { return _completion.load() == Completion::FinishedExecuting; }
    auto has_been_canceled() const -> bool { return _completion.load() == Completion::Canceled; }

    auto name() const -> std::string const& { return _name; }

private:
    friend class TaskManager;

    /// When the user tries to close the app, should we warn them that this task has not started and/or finished executing, and would be canceled?
    virtual auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool = 0;

    /// Do the actual work of the task
    /// NB: don't put cleanup work in execute(), it might never be called because the coroutine might be cancelled and never run all the way to its end
    virtual auto execute() -> TaskCoroutine = 0;

    /// Called as soon as the task is given to task_manager().submit()
    virtual void on_submit() {}

    /// Always called (if the task has been submitted to the TaskManager), either after execute() is finished, or after canceling the task. on_submit() is guaranteed to have been called before. This is called only once
    /// You shouldn't put your cleanup logic in the destructor, because the task might be kept alive by someone referencing it long after it has been canceled / has finished executing
    virtual void cleanup_impl(bool /* has_been_canceled */) {}

private:
    /// Request to finish the task as quickly as possible
    void cancel() { _completion.store(Completion::Canceled); } // Once all tasks properly implement suspending regularly, we can remove it (save image, http request)
    void cleanup(bool has_been_canceled);

private:
    reg::AnyId  _owner_id{};
    std::string _name;

    enum class Completion : uint8_t {
        None,
        Canceled,
        FinishedExecuting,
    };
    std::atomic<Completion> _completion{Completion::None};
};

} // namespace Cool