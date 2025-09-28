#include "copy_tex_pipeline.hpp"

namespace Cool {

static auto make_copy_tex_pipeline() -> Cool::OpenGL::FullscreenPipeline
{
    auto instance = Cool::OpenGL::FullscreenPipeline{};
    instance.compile(R"GLSL(
#version 410
#include "_COOL_RES_/shaders/shader-utils.glsl" 
out vec4 out_Color;
uniform sampler2D tex_to_copy;
void main()
{
    out_Color = texture(tex_to_copy, _uv);
}
    )GLSL")
        .or_else([](ErrorMessage const& err) {
            Log::internal_error("make_copy_tex_pipeline", fmt::format("Failed to create shader to copy texture:\n{}", err.message), err.clipboard_contents);
        });

    return instance;
}

auto copy_tex_pipeline() -> Cool::OpenGL::FullscreenPipeline&
{
    static auto instance = make_copy_tex_pipeline();
    return instance;
}

} // namespace Cool