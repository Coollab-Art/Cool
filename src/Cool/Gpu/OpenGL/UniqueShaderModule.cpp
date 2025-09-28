#if defined(COOL_OPENGL)
#include "UniqueShaderModule.h"

namespace Cool {

static GLenum opengl_shader_kind(ShaderKind kind)
{
    switch (kind)
    {
    case ShaderKind::Vertex:
        return GL_VERTEX_SHADER;
    case ShaderKind::Fragment:
        return GL_FRAGMENT_SHADER;
    case ShaderKind::Geometry:
        return GL_GEOMETRY_SHADER;
    case ShaderKind::Compute:
        return GL_COMPUTE_SHADER;
    }
    assert(false);
    return 0;
}

UniqueShaderModule::UniqueShaderModule(ShaderKind shader_kind)
{
    GLDebug(_id = glCreateShader(opengl_shader_kind(shader_kind)));
}

UniqueShaderModule::~UniqueShaderModule()
{
    GLDebug(glDeleteShader(_id));
}

UniqueShaderModule::UniqueShaderModule(UniqueShaderModule&& rhs) noexcept
    : _id{rhs._id}
{
    rhs._id = decltype(rhs._id){};
}

UniqueShaderModule& UniqueShaderModule::operator=(UniqueShaderModule&& rhs) noexcept
{
    if (&rhs != this)
    {
        GLDebug(glDeleteShader(_id));
        _id     = rhs._id;
        rhs._id = decltype(rhs._id){};
    }
    return *this;
}

} // namespace Cool

#endif