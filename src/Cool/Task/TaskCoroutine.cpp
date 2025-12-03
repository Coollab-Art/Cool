#include "TaskCoroutine.hpp"
#include "TaskManager.hpp"

namespace Cool {

auto SuspendTask::await_ready() const noexcept -> bool
{
    return true;
    return !task_manager().is_saturated();
}

} // namespace Cool