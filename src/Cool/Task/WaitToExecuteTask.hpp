#pragma once
#include <chrono>
#include <memory>
#include "Cool/Utils/Delay.hpp"
#include "Task.hpp"

namespace Cool {

class WaitToExecuteTask {
public:
    WaitToExecuteTask()                                        = default;
    virtual ~WaitToExecuteTask()                               = default;
    WaitToExecuteTask(WaitToExecuteTask const&)                = delete;
    WaitToExecuteTask& operator=(WaitToExecuteTask const&)     = delete;
    WaitToExecuteTask(WaitToExecuteTask&&) noexcept            = delete;
    WaitToExecuteTask& operator=(WaitToExecuteTask&&) noexcept = delete;

    virtual auto wants_to_execute() -> bool = 0;
    virtual auto wants_to_cancel() -> bool  = 0;
};

class WaitToExecuteTask_Nothing : public WaitToExecuteTask {
public:
    auto wants_to_execute() -> bool override { return true; }
    auto wants_to_cancel() -> bool override { return false; }
};

class WaitToExecuteTask_Delay : public WaitToExecuteTask {
public:
    explicit WaitToExecuteTask_Delay(std::chrono::milliseconds delay)
        : _delay{delay}
    {}

    auto wants_to_execute() -> bool override { return _delay.has_expired(); }
    auto wants_to_cancel() -> bool override { return false; }

private:
    Delay _delay;
};

class WaitToExecuteTask_OtherTask : public WaitToExecuteTask {
public:
    explicit WaitToExecuteTask_OtherTask(std::shared_ptr<Task> const& task)
        : _task{task}
    {}

    auto wants_to_execute() -> bool override { return _task->has_been_executed(); }
    auto wants_to_cancel() -> bool override { return _task->has_been_canceled(); }

private:
    std::shared_ptr<Task> _task;
};

class WaitToExecuteTask_AllOf : public WaitToExecuteTask {
public:
    WaitToExecuteTask_AllOf(std::shared_ptr<WaitToExecuteTask> const& a, std::shared_ptr<WaitToExecuteTask> const& b) // NOLINT(*easily-swappable-parameters)
        : _a{a}
        , _b{b}
    {}

    auto wants_to_execute() -> bool override { return _a->wants_to_execute() && _b->wants_to_execute(); }
    auto wants_to_cancel() -> bool override { return _a->wants_to_cancel() || _b->wants_to_cancel(); }

private:
    std::shared_ptr<WaitToExecuteTask> _a;
    std::shared_ptr<WaitToExecuteTask> _b;
};

class WaitToExecuteTask_AnyOf : public WaitToExecuteTask {
public:
    WaitToExecuteTask_AnyOf(std::shared_ptr<WaitToExecuteTask> const& a, std::shared_ptr<WaitToExecuteTask> const& b) // NOLINT(*easily-swappable-parameters)
        : _a{a}
        , _b{b}
    {}

    auto wants_to_execute() -> bool override { return _a->wants_to_execute() || _b->wants_to_execute(); }
    auto wants_to_cancel() -> bool override { return _a->wants_to_cancel() && _b->wants_to_cancel(); }

private:
    std::shared_ptr<WaitToExecuteTask> _a;
    std::shared_ptr<WaitToExecuteTask> _b;
};

} // namespace Cool

// In global namespace so that it reads nicer (`Cool::task_manager().submit(after(1s) && after(other_task), ...)`)

inline auto after_nothing() -> std::shared_ptr<Cool::WaitToExecuteTask>
{
    return std::make_shared<Cool::WaitToExecuteTask_Nothing>();
}

inline auto after(std::chrono::milliseconds delay) -> std::shared_ptr<Cool::WaitToExecuteTask>
{
    return std::make_shared<Cool::WaitToExecuteTask_Delay>(delay);
}

inline auto after(std::shared_ptr<Cool::Task> const& task) -> std::shared_ptr<Cool::WaitToExecuteTask>
{
    return std::make_shared<Cool::WaitToExecuteTask_OtherTask>(task);
}

inline auto operator&&(std::shared_ptr<Cool::WaitToExecuteTask> const& a, std::shared_ptr<Cool::WaitToExecuteTask> const& b) -> std::shared_ptr<Cool::WaitToExecuteTask>
{
    return std::make_shared<Cool::WaitToExecuteTask_AllOf>(a, b);
}

inline auto operator||(std::shared_ptr<Cool::WaitToExecuteTask> const& a, std::shared_ptr<Cool::WaitToExecuteTask> const& b) -> std::shared_ptr<Cool::WaitToExecuteTask>
{
    return std::make_shared<Cool::WaitToExecuteTask_AnyOf>(a, b);
}
