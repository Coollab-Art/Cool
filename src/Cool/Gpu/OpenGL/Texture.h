#if defined(COOL_OPENGL)
#pragma once
#include <img/src/SizeU.h>
#include <glpp/glpp.hpp>
#include "TextureRef.hpp"
#include "glpp-extended/src/Texture2D.h"
#include "img/src/Image.h"
#include "img/src/Size.h"

namespace Cool::OpenGL {

struct TextureConfig {
    glpp::Interpolation interpolation_mode{glpp::Interpolation::Linear};
    glpp::Wrap          wrap_mode{glpp::Wrap::ClampToEdge};
};

/// A wrapper for an OpenGL texture2D.
class Texture {
public:
    /// Creates an empty texture.
    explicit Texture(TextureConfig = {});
    /// Create a texture with the given size and no pixel data.
    explicit Texture(img::Size const&, TextureConfig = {});
    /// Create a texture containing a copy of the image's data.
    explicit Texture(img::Image const&, bool need_to_flip_y = false, TextureConfig = {});
    /// Create a texture containing a copy of data.
    /// `channels_count` should be 3 for RGB and 4 for RGBA.
    Texture(img::Size const&, size_t channels_count, uint8_t const* data, bool need_to_flip_y = false, TextureConfig = {});
    Texture(img::Size const&, uint8_t const* data, glpp::TextureLayout const&, bool need_to_flip_y = false, TextureConfig = {});

    void set_size(img::Size const&);
    void set_image(img::Image const&, bool need_to_flip_y = false);
    /// `channels_count` should be 3 for RGB and 4 for RGBA.
    void set_image(img::Size const&, size_t channels_count, uint8_t const* data, bool need_to_flip_y = false);
    void set_image(img::Size const&, uint8_t const* data, glpp::TextureLayout const&, bool need_to_flip_y = false);
    void set_interpolation_mode(glpp::Interpolation interpolation_mode);
    void set_wrap_mode(glpp::Wrap wrap_mode);

    void        bind();
    static void unbind();

    [[nodiscard]] auto ref() const -> TextureRef { return {id(), size()}; }
    /// The native OpenGL ID of the texture.
    [[nodiscard]] auto id() const -> GLuint { return _tex.id(); }
    /// The ID that ImGui expects
    [[nodiscard]] auto imgui_texture_id() const -> ImTextureID { return static_cast<ImTextureID>(id()); }

    [[nodiscard]] auto aspect_ratio() const -> float { return img::aspect_ratio(_size); }
    [[nodiscard]] auto size() const -> img::Size { return _size; }
    void               bob_size(unsigned int w, unsigned int h)
    {
        _size.set_width(w);
        _size.set_height(h);
    }

    [[nodiscard]] auto need_to_flip_y() const -> bool { return _need_to_flip_y; }

    /// Attaches your texture to a slot, so that it is ready to be read by a shader.
    /// This `slot` is the value that you should also send to your shader as the value of a "uniform sampler2D u_YourTextureUniform"
    void attach_to_slot(GLuint slot) const;

private:
    glpp::Texture2D _tex{};
    img::Size       _size{};
    bool            _need_to_flip_y{false};
    bool            _data_has_been_uploaded{false};
};

} // namespace Cool::OpenGL

#endif