#include "gen_dump_string.h"
#include <Cool/Gpu/gpu_info.h>
#include <Cool/Path/Path.h>
#include <os_name/os_name.hpp>
#include <string>
#include "app_version.hpp"

namespace Cool {

// TODO(Logs) make all the dumps align

auto gen_dump_string() -> std::string
{
    return fmt::format(
        R"STR(---INFO---
{} : {}
OS              : {}
GPU API         : {}
----------)STR",
        COOL_APP_NAME, app_version(),
        os_name(),
        full_gpu_info() // TODO(WebGPU) Check if we can access the info even if we failed to init WebGPU // And check that it shows the underlying chosen GPU API
    );
}

} // namespace Cool