#include <glpp/UniqueHandles/UniqueTexture.h>
#if defined(COOL_OPENGL)

#include "TextureFB.h"

namespace Cool {

static auto get_format(TextureFormat format) -> GLenum
{
    switch (format.num_components)
    {
    case 1:
        return GL_RED;
        {
        case 2:
            return GL_RG;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        }
    }
    assert(false);
    return GL_RGBA;
}

static auto get_type(TextureFormat format) -> GLenum
{
    switch (format.type)
    {
    case Cool::PixelType::Float16:
    case Cool::PixelType::Float32:
        return GL_FLOAT;
    case Cool::PixelType::UInt8:
        return GL_UNSIGNED_BYTE;
    }
    assert(false);
    return GL_UNSIGNED_BYTE;
}

static auto get_internal_format(TextureFormat format) -> GLint
{
    if (format.num_components == 4 && format.type == PixelType::Float32)
        return GL_RGBA32F;
    if (format.num_components == 4 && format.type == PixelType::Float16)
        return GL_RGBA16F;
    if (format.num_components == 4 && format.type == PixelType::UInt8)
        return GL_RGBA8;
    if (format.num_components == 3 && format.type == PixelType::UInt8)
        return GL_RGB8;
    if (format.num_components == 1 && format.type == PixelType::UInt8)
        return GL_R8;
    assert(false);
    return GL_RGBA8;
}

void TextureFB::createAttachments(img::Size size)
{
    FrameBuffer::createAttachments(size);
    m_colorTextureId.emplace();
    GLDebug(glBindTexture(GL_TEXTURE_2D, m_colorTextureId->id()));
    GLDebug(glTexImage2D(GL_TEXTURE_2D, 0, get_internal_format(_format), static_cast<GLsizei>(size.width()), static_cast<GLsizei>(size.height()), 0, get_format(_format), get_type(_format), nullptr));
    GLDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));  // TODO should be nearest ?
    GLDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)); // TODO should be linear ?
    GLDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLDebug(glBindTexture(GL_TEXTURE_2D, 0));
}

void TextureFB::destroyAttachments()
{
    FrameBuffer::destroyAttachments();
    m_colorTextureId.reset();
}

void TextureFB::attachAttachments()
{
    FrameBuffer::attachAttachments();
    GLDebug(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId()));
    GLDebug(glBindTexture(GL_TEXTURE_2D, m_colorTextureId->id()));
    GLDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextureId->id(), 0));
    GLDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void TextureFB::attachTextureToSlot(unsigned int slot) const
{
    GLDebug(glActiveTexture(GL_TEXTURE0 + slot));
    GLDebug(glBindTexture(GL_TEXTURE_2D, m_colorTextureId->id()));
}

} // namespace Cool

#endif