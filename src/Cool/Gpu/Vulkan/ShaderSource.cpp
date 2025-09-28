#if defined(COOL_VULKAN)

#include "ShaderSource.h"

static shaderc_shader_kind shader_kind_cool2shaderc(Cool::ShaderKind shader_kind)
{
    switch (shader_kind)
    {
    case Cool::ShaderKind::Vertex:
        return shaderc_shader_kind::shaderc_vertex_shader;
    case Cool::ShaderKind::Fragment:
        return shaderc_shader_kind::shaderc_fragment_shader;
    case Cool::ShaderKind::Geometry:
        return shaderc_shader_kind::shaderc_geometry_shader;
    case Cool::ShaderKind::Compute:
        return shaderc_shader_kind::shaderc_compute_shader;
    }
    assert(false);
    return shaderc_shader_kind::shaderc_vertex_shader;
}

namespace Cool::Vulkan::ShaderSource {

shaderc::SpvCompilationResult to_spirv(const ShaderDescription& shader_description)
{
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.AddMacroDefinition("COOL_VULKAN");

    auto res = compiler.CompileGlslToSpv(shader_description.source_code, shader_kind_cool2shaderc(shader_description.kind), "Unknown", options);

    if (!res.GetErrorMessage().empty())
        Log::internal_warning(res.GetErrorMessage());

    return res;
}

} // namespace Cool::Vulkan::ShaderSource

#endif