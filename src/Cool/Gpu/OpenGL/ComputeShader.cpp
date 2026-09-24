#if defined(COOL_OPENGL)

#include "ComputeShader.h"
#include <filesystem>
#include <glm/glm.hpp>
#include "Cool/File/File.h"
#include "Shader.h"
#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/vector_relational.hpp"

namespace Cool::OpenGL {

static void assert_compute_shader_is_bound(GLuint id)
{
#if DEBUG
    GLint current_id = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_id);
    assert(static_cast<GLuint>(current_id) == id && "The compute shader must be bound before calling compute().");
#else
    (void)id;
#endif
}

ComputeShader::ComputeShader(glm::uvec3 working_group_size, std::string_view source_code)
    : Shader(
        ShaderModule{ShaderDescription{
            .kind        = ShaderKind::Compute,
            .source_code = generate_boilerplate_for_size(working_group_size) + std::string(source_code),
        }}
    )
    , _working_group_size(working_group_size)
{}

ComputeShader::ComputeShader(glm::uvec2 working_group_size, std::string_view source_code)
    : ComputeShader(glm::uvec3(working_group_size, 1), source_code) {}
ComputeShader::ComputeShader(unsigned int working_group_size, std::string_view source_code)
    : ComputeShader(glm::uvec3(working_group_size, 1, 1), source_code) {}

void ComputeShader::compute(glm::uvec3 size)
{
    if (size.x == 0 || size.y == 0 || size.z == 0)
        return; // There is nothing to compute. We must return early, otherwise the unsigned subtraction below underflows and we dispatch billions of work groups (which triggers a GPU timeout reset on Windows).
    assert_compute_shader_is_bound(id());
    set_uniform("DispatchSize", size);
    glm::uvec3 dispatch_group_count = (size - glm::uvec3{1}) / _working_group_size + glm::uvec3(1);
    GLDebug(glDispatchCompute(dispatch_group_count.x, dispatch_group_count.y, dispatch_group_count.z));
    GLDebug(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));
}

auto load_compute_shader_from_file(glm::uvec3 working_group_size, const std::filesystem::path& filePath)
    -> ComputeShader
{
    return ComputeShader(working_group_size, *File::to_string(filePath));
}

auto generate_boilerplate_for_size(glm::uvec3 working_group_size) -> std::string
{
    return std::string(R"V0G0N(
#version 430
#define COOL_COMPUTE_SHADER // Used to know if we can use some functions that are specific to compute shaders / fragment shaders.

uniform uvec3 DispatchSize;

void cool_main();

)V0G0N")
           + "layout(local_size_x = "
           + std::to_string(working_group_size.x)
           + ", local_size_y = "
           + std::to_string(working_group_size.y)
           + ", local_size_z = "
           + std::to_string(working_group_size.z)
           + ") in;"
           + R"V0G0N(
void main() {
    if (gl_GlobalInvocationID.x < DispatchSize.x
     && gl_GlobalInvocationID.y < DispatchSize.y
     && gl_GlobalInvocationID.z < DispatchSize.z
    ) {
        cool_main();
    }
}
)V0G0N";
};

} // namespace Cool::OpenGL

#endif