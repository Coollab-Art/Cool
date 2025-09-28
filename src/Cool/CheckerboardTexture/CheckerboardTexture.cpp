#include "CheckerboardTexture.hpp"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/Gpu/FullscreenPipeline.h"

namespace Cool {

static auto create_alpha_checkerboard_pipeline() -> FullscreenPipeline
{
    auto pipeline = FullscreenPipeline{};
    pipeline.compile(R"STR(#version 410
out vec4 out_Color;
layout(location = 0) in vec2 _uv;
uniform float _aspect_ratio;

void main()
{
    vec2 uv = _uv - 0.5;
    uv.x *= _aspect_ratio;
    ivec2 gid = ivec2(floor(uv * 20.));
    float grey = ((gid.x + gid.y) % 2 == 0) ? 0.25 : 0.5;
    out_Color = vec4(vec3(grey), 1.);
}
)STR")
        .or_else([](ErrorMessage const& err) {
            Log::internal_error("Alpha Checkerboard Shader", err.message, err.clipboard_contents);
        });

    return pipeline;
}

static auto alpha_checkerboard_pipeline() -> FullscreenPipeline const&
{
    static auto pipeline = create_alpha_checkerboard_pipeline();
    return pipeline;
}

static void rerender_alpha_checkerboard_ifn(img::Size size, RenderTarget& render_target)
{
    if (size == render_target.current_size())
        return;

    render_target.set_size(size);
    render_target.render([&]() {
        alpha_checkerboard_pipeline().shader()->bind();
        alpha_checkerboard_pipeline().shader()->set_uniform("_aspect_ratio", img::aspect_ratio(size));
        alpha_checkerboard_pipeline().draw();
    });

    if (Cool::DebugOptions::log_when_rendering_alpha_checkerboard_background())
        Log::info("Alpha Checkerboard", "Rendered");
}

auto CheckerboardTexture::get(img::Size size) -> RenderTarget const&
{
    rerender_alpha_checkerboard_ifn(size, _render_target);
    return _render_target;
}

} // namespace Cool