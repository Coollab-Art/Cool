#include "gen_dump_string.hpp"
#include <string>
#include "DumpStringGenerator.hpp"
#include "app_version.hpp"
#include "gpu_api_version.hpp"
#include "os_name/os_name.hpp"
#include "set_extra_dump_info.hpp"

namespace Cool {

auto gen_dump_string() -> std::string
{
    auto dump = DumpStringGenerator{};
    dump.add(COOL_APP_NAME, app_version())
        .add("OS", os_name())
        // This is the version we *request*, not the one the driver gives us. The actual GPU, driver and context version are logged by log_opengl_info(): they can't go here, because this string is written to the log file before any GPU context exists.
        .add("Requested GPU API", gpu_api_version())
        .add("Build mode",
#if DEBUG
             "Debug"
#else
             "Release"
#endif
        );
    if (get_extra_dump_info())
        get_extra_dump_info()(dump);
    return dump.generate();
}

} // namespace Cool