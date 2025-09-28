#include "close_application.hpp"
#include "internal/get_app_manager.hpp"

namespace Cool {

void close_application()
{
    if (!internal::get_app_manager())
    {
        assert(false);
        return;
    }
    internal::get_app_manager()->close_application();
}

void close_application_if_all_tasks_are_done()
{
    if (!internal::get_app_manager())
    {
        assert(false);
        return;
    }
    internal::get_app_manager()->close_application_if_all_tasks_are_done();
}

} // namespace Cool