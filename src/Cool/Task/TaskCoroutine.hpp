#pragma once
#include <coroutine>

namespace Cool {

class TaskCoroutine {
public:
    TaskCoroutine(TaskCoroutine const&)            = delete;
    TaskCoroutine& operator=(TaskCoroutine const&) = delete;
    TaskCoroutine(TaskCoroutine&& rhs) noexcept
        : _handle{std::exchange(rhs._handle, nullptr)}
    {}
    TaskCoroutine& operator=(TaskCoroutine&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        if (_handle)
            _handle.destroy();
        _handle     = rhs._handle;
        rhs._handle = nullptr;
        return *this;
    }
    ~TaskCoroutine()
    {
        if (_handle)
            _handle.destroy();
    }

    void do_some_work()
    {
        assert(!_handle.done());
        _handle.resume();
    }

    auto has_finished() const -> bool
    {
        return _handle.done();
    }

public:
    struct promise_type {
        void unhandled_exception() { throw; } // TODO(Task) catch this exception in TaskManager
        auto get_return_object() { return TaskCoroutine{*this}; }
        auto initial_suspend() noexcept -> std::suspend_always { return {}; }
        void return_void() noexcept {}
        auto final_suspend() noexcept -> std::suspend_always { return {}; }
    };
    explicit TaskCoroutine(promise_type& p)
        : _handle{Handle::from_promise(p)}
    {}

private:
    using Handle = std::coroutine_handle<promise_type>;
    Handle _handle;
};

using TaskCoroutine = std::coroutine_handle<promise_type>;

// et les deux méthodes je peux juste les appeler raw sur la handle dans le TaskManager, et comme ça je peux schedule + facilement la handle ? Modulo j'ai encore la tâche associée à trouver

struct SuspendTask {
    // auto operator co_await() -> std::suspend_always { return {}; } // TODO(Task) Only suspend if there are other tasks in the TaskManager queue that are waiting to be processed?

    // Do we really need to suspend?
    auto await_ready() const noexcept -> bool;

    // Schedule the coroutine, or destroy it
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

    constexpr void await_resume() const noexcept {}
};

struct SuspendFor {
    auto await_ready() const noexcept -> bool { return false; }
};

} // namespace Cool