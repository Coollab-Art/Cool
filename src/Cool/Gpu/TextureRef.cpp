#include "TextureRef.hpp"
#include "Cool/Gpu/OpenGL/copy_tex_pipeline.hpp"
#include "Cool/Gpu/RenderTarget.h"
#include "glpp/glpp.hpp"

namespace Cool {

auto TextureRef::download_pixels() const -> img::Image
{
    static auto rt = RenderTarget{};
    rt.set_size(size);
    std::unique_ptr<uint8_t[]> data{new uint8_t[4 * width() * height()]};
    rt.render([&]() {
        // TODO(WebGPU) We shouldn't need to run a shader to get the texture on a framebuffer and then download its pixels
        Cool::copy_tex_pipeline().shader()->bind();
        Cool::copy_tex_pipeline().shader()->set_uniform_texture("tex_to_copy", id);
        glDisable(GL_BLEND);
        Cool::copy_tex_pipeline().draw();
        glEnable(GL_BLEND);
        glReadPixels(0, 0, static_cast<GLsizei>(width()), static_cast<GLsizei>(height()), GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    });
    return img::Image{
        img::Size{width(), height()},
        img::PixelFormat::RGBA,
        img::FirstRowIs::Bottom,
        data.release()
    };
}

} // namespace Cool