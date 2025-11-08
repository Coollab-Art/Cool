#if defined(COOL_OPENGL)

#include "RenderTarget_ImplOpenGL.h"

namespace Cool {

RenderTarget_ImplOpenGL::RenderTarget_ImplOpenGL(TextureFormat format, img::Size size)
    : _texture_fb{format}
{
    resize(size);
}

void RenderTarget_ImplOpenGL::render(RenderFuncType render_fn)
{
    _texture_fb.bind();
    render_fn();
    _texture_fb.unbind();
}

RenderTargetInfo RenderTarget_ImplOpenGL::info() const
{
    return RenderTargetInfo{
        .viewport = {
            .size               = size(),
            .bottom_left_corner = {0, 0}
        }
    };
}

void RenderTarget_ImplOpenGL::resize(img::Size size)
{
    _texture_fb.setSize(size);
}

img::Image RenderTarget_ImplOpenGL::download_pixels() const
{
    _texture_fb.bind();
    std::unique_ptr<uint8_t[]> data{new uint8_t[4 * width() * height()]};
    glReadPixels(0, 0, static_cast<GLsizei>(width()), static_cast<GLsizei>(height()), GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    _texture_fb.unbind();
    return img::Image{
        img::Size{width(), height()},
        img::PixelFormat::RGBA,
        img::FirstRowIs::Bottom,
        data.release()
    };
}

} // namespace Cool

#endif