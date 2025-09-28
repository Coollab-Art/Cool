#pragma once
#include <ShaderTranspiler/ShaderTranspiler.hpp>
#include <tl/expected.hpp>
#include "Cool/Log/OptionalErrorMessage.h"
#include "FullscreenPipeline.h"

namespace Cool {

class FullscreenPipelineGLSL : public FullscreenPipeline {
public:
    explicit FullscreenPipelineGLSL(FullscreenPipeline_CreationArgs const& args, std::vector<shadert::Uniform> uniforms_locations);

    template<typename T>
    void set_uniform_with_name(std::string_view name, T const& value) const
    {
        for (auto const& uniform : _uniforms_locations)
        {
            if (uniform.name == name)
            {
                set_uniform_with_offset(uniform.bufferOffset, value);
                return;
            }
        }
        // TODO(WebGPU) if a debug option is enabled, warn that uniform was not set ?
        // TODO(WebGPU) DebugOption to rerender every frame
        // TODO(WebGPU) DebugOption to rerender once
    }

private:
    std::vector<shadert::Uniform> _uniforms_locations{};
};

auto make_fullscreen_pipeline_glsl(FullscreenPipeline_CreationArgs args) -> tl::expected<FullscreenPipelineGLSL, ErrorMessage>;

} // namespace Cool
