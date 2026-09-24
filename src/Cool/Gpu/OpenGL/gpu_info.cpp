#if defined(COOL_OPENGL)
#include "gpu_info.h"

namespace Cool {

/// glGetString() returns nullptr if there is no current context, or if it errors out.
static auto get_string(GLenum name) -> char const*
{
    char const* str = reinterpret_cast<char const*>(glGetString(name));
    return str ? str : "<unknown>";
}

auto full_gpu_info_impl() -> std::string
{
    return fmt::format(R"STR(GPU vendor         : {}
GPU model          : {}
GPU driver         : {})STR",
                       get_string(GL_VENDOR), get_string(GL_RENDERER), get_string(GL_VERSION));
}

auto gpu_name_impl() -> std::string
{
    return get_string(GL_RENDERER);
}

} // namespace Cool

#endif