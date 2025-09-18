#include "Task.hpp"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/Log/Log.hpp"

namespace Cool {

Task::~Task()
{
    if (Cool::DebugOptions::log_tasks())
    {
        if (_completion.load() == Completion::None)
            Cool::Log::info(_name, "Task has never been submitted to the TaskManager. Or there is a bug in the TaskManager and we forgot to call cleanup()");
    }
}

void Task::cleanup(bool has_been_canceled)
{
    if (has_been_canceled && Cool::DebugOptions::log_tasks())
        Cool::Log::info(_name, "Task Cancelled");

    cleanup_impl(has_been_canceled);

    _completion.store(has_been_canceled ? Completion::Canceled : Completion::FinishedExecuting);
}

} // namespace Cool