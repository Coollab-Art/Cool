#include "FullscreenPipelineGLSL.h"
#include <tl/expected.hpp>
#include "FullscreenPipeline.h"
#include "preprocess_shader_source.h"

namespace Cool {

auto make_fullscreen_pipeline_glsl(FullscreenPipeline_CreationArgs args) -> tl::expected<FullscreenPipelineGLSL, ErrorMessage>
{
    auto const preprocessed = preprocess_shader_source(args.fragment_shader_module_creation_args.code);
    if (!preprocessed)
    {
        return tl::make_unexpected(
            ErrorMessage{
                fmt::format("Preprocessing failed:\n{}\n\nThe source code we tried to preprocess was:\n{}", preprocessed.error(), args.fragment_shader_module_creation_args.code),
                std::vector<ClipboardContent>{
                    {.title = "shader code", .content = std::string{args.fragment_shader_module_creation_args.code}},
                    {.title = "error message", .content = preprocessed.error()},
                }
            }
        );
    }

    shadert::ShaderTranspiler  s;
    shadert::MemoryCompileTask task{preprocessed.value(), shadert::ShaderStage::Fragment};
    shadert::Options           opt;
    opt.enableInclude   = false;
    opt.debug           = false;
    opt.entryPoint      = "main";
    opt.preambleContent = "";
    // opt.uniformBufferSettings
    opt.mobile  = false; // used for OpenGL ES or Metal iOS
    opt.version = 13;    // stores the major and minor version, for Vulkan 1.5 use 15

    try
    {
        shadert::CompileResult result = s.CompileTo(task, shadert::TargetAPI::WGSL, opt);
        std::cout << result.data.sourceData << '\n';
        args.fragment_shader_module_creation_args.code = std::move(result.data.sourceData);
        return FullscreenPipelineGLSL{args, std::move(result.data.uniformData)};
    }
    catch (std::exception& e) // The transpiler will throw on errors
    {
        return tl::make_unexpected(
            ErrorMessage{
                fmt::format("Compilation failed:\n{}The source code we tried to compile was:\n{}", e.what(), *preprocessed),
                std::vector<ClipboardContent>{
                    {.title = "shader code", .content = *preprocessed},
                    {.title = "error message", .content = e.what()},
                }
            }
        );
    }
}

FullscreenPipelineGLSL::FullscreenPipelineGLSL(FullscreenPipeline_CreationArgs const& args, std::vector<shadert::Uniform> uniforms_locations)
    : FullscreenPipeline{args}
    , _uniforms_locations{std::move(uniforms_locations)}
{
}

} // namespace Cool
