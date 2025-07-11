#include "gpu_api_version.hpp"
#include "Cool/Utils/Version.h"

namespace Cool {

auto gpu_api_version() -> std::string
{
    static constexpr auto format_version = [](const char* api_name, int version) {
        return fmt::format("{} {}.{}", api_name, Cool::major_version(version), Cool::minor_version(version));
    };

#if COOL_OPENGL
    return format_version("OpenGL", COOL_OPENGL_VERSION);
#elif COOL_VULKAN
    return format_version("Vulkan", COOL_VULKAN_VERSION);
#else
#error Unknown GPU API
#endif
}

} // namespace Cool