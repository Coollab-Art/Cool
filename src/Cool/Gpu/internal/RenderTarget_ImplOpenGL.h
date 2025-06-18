#pragma once
#if defined(COOL_OPENGL)
#include <img/img.hpp>
#include "../OpenGL/Texture.h"
#include "../OpenGL/TextureFB.h"
#include "../RenderTargetInfo.h"

namespace Cool {

class RenderTarget_ImplOpenGL {
public:
    using RenderFuncType = std::function<void()>;
    RenderTarget_ImplOpenGL(img::Size size = {});
    void render(RenderFuncType render_fn);

    RenderTargetInfo    info() const;
    img::Size::DataType width() const { return _texture_fb.width(); }
    img::Size::DataType height() const { return _texture_fb.height(); }
    img::Size           size() const { return _texture_fb.size(); }
    img::Image          download_pixels() const;
    GLuint              texture_id() const { return _texture_fb.textureID(); }
    auto                texture_ref() const -> TextureRef { return TextureRef{texture_id(), size()}; }
    ImTextureID         imgui_texture_id() const { return static_cast<ImTextureID>(_texture_fb.textureID()); }

    void imgui_window();

    void resize(img::Size size);

private:
    TextureFB _texture_fb;
};

} // namespace Cool

#endif