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
        .add("GPU API", gpu_api_version()) //  full_gpu_info(), // TODO(WebGPU) Check if we can access the info even if we failed to init WebGPU // And check that it shows the underlying chosen GPU API
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