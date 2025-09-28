#if defined(COOL_OPENGL)
#include "FrameBuffer.h"

namespace Cool {

void FrameBuffer::setSize(img::Size size)
{
    m_size = size;
    destroyAttachments();
    createAttachments(size);
    attachAttachments();
#if DEBUG
    bind();
    GLDebug(auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        const auto statusStr = [status]() {
            switch (status)
            {
            case GL_FRAMEBUFFER_UNDEFINED:
                return "GL_FRAMEBUFFER_UNDEFINED";
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT ";
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT ";
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER ";
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER ";
            case GL_FRAMEBUFFER_UNSUPPORTED:
                return "GL_FRAMEBUFFER_UNSUPPORTED ";
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE ";
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS ";
            default:
                return "UNKNOWN_ERROR";
            }
        }();
        Log::internal_error("Framebuffer incomplete", statusStr);
    }
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Make sure to init the values in the framebuffer
    unbind();
#endif
}

void FrameBuffer::bind() const
{
    GLDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId.id()));
    GLDebug(glGetIntegerv(GL_VIEWPORT, m_prevViewport));                                      // Store viewport settings to restore them when unbinding
    GLDebug(glViewport(0, 0, static_cast<GLsizei>(width()), static_cast<GLsizei>(height()))); // Only useful if we plan on using this frame buffer at a different resolution than the screen's
}

void FrameBuffer::unbind() const
{
    GLDebug(glBindFramebuffer(GL_FRAMEBUFFER, SCREEN_FRAMEBUFFER_ID));
    GLDebug(glViewport(m_prevViewport[0], m_prevViewport[1], m_prevViewport[2], m_prevViewport[3]));
}

void FrameBuffer::blitTo(const glm::ivec2& botLeft, const glm::ivec2& topRight, GLuint dstFrameBufferID, GLenum interpolationMode) const
{
    GLDebug(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFrameBufferID));
    GLDebug(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBufferId.id()));
    GLDebug(glBlitFramebuffer(0, 0, static_cast<GLsizei>(width()), static_cast<GLsizei>(height()), botLeft.x, botLeft.y, topRight.x, topRight.y, GL_COLOR_BUFFER_BIT, interpolationMode));
    GLDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::blitTo(FrameBuffer& frameBuffer, GLenum interpolationMode) const
{
    blitTo({0, 0}, {width(), height()}, frameBuffer.frameBufferId(), interpolationMode);
}

void FrameBuffer::createAttachments(img::Size size)
{
    m_depthRenderBufferId.emplace();
    GLDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBufferId->id()));
    GLDebug(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(size.width()), static_cast<GLsizei>(size.height())));
    GLDebug(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void FrameBuffer::destroyAttachments()
{
    m_depthRenderBufferId.reset();
}

void FrameBuffer::attachAttachments()
{
    GLDebug(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId()));
    GLDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBufferId->id()));
    GLDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

} // namespace Cool

#endif