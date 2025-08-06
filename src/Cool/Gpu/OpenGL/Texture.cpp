#include <glpp-extended/src/TextureLayout.h>
#if defined(COOL_OPENGL)
#include "../Texture.h"
#include "glpp-extended/src/ImageSize.h"
#include "glpp/Enums/Channels.h"
#include "glpp/Enums/InternalFormat.h"
#include "glpp/Enums/TexelDataType.h"
#include "glpp/Enums/TextureKind.h"
#include "glpp/Functions/Texture.h"
#include "img/img.hpp"
#include "img/src/Size.h"

namespace Cool::OpenGL {

Texture::Texture(TextureConfig config)
{
    set_wrap_mode(config.wrap_mode);
    set_interpolation_mode(config.interpolation_mode);
}

Texture::Texture(img::Size const& size, TextureConfig config)
    : Texture{config}
{
    set_size(size);
}

Texture::Texture(img::Image const& image, bool need_to_flip_y, TextureConfig config)
    : Texture{config}
{
    set_image(image, need_to_flip_y);
}

Texture::Texture(img::Size const& size, size_t channels_count, uint8_t const* data, bool need_to_flip_y, TextureConfig config)
    : Texture{config}
{
    set_image(size, channels_count, data, need_to_flip_y);
}

Texture::Texture(img::Size const& size, uint8_t const* data, glpp::TextureLayout const& layout, bool need_to_flip_y, TextureConfig config)
    : Texture{config}
{
    set_image(size, data, layout, need_to_flip_y);
}

static auto img_to_glpp_size(img::Size const& size) -> glpp::ImageSize
{
    return {static_cast<GLsizei>(size.width()), static_cast<GLsizei>(size.height())};
}

void Texture::set_size(img::Size const& size)
{
    _size = size;
    _tex.resize(img_to_glpp_size(size));
    _data_has_been_uploaded = true;
}

auto Texture::set_size_ifn(img::Size const& size) -> bool
{
    if (_data_has_been_uploaded && _size == size)
        return false;
    set_size(size);
    return true;
}

void Texture::set_image(img::Image const& img, bool need_to_flip_y)
{
    set_image(img.size(), img.channels_count(), img.data(), need_to_flip_y);
}

void Texture::set_image(img::Size const& size, size_t channels_count, uint8_t const* data, bool need_to_flip_y)
{
    assert(channels_count == 3 || channels_count == 4);
    set_image(
        size,
        data,
        {
            .internal_format = channels_count == 3 ? glpp::InternalFormat::RGB : glpp::InternalFormat::RGBA,
            .channels        = channels_count == 3 ? glpp::Channels::RGB : glpp::Channels::RGBA,
            .texel_data_type = glpp::TexelDataType::UnsignedByte,
        },
        need_to_flip_y
    );
}

void Texture::set_image(img::Size const& size, uint8_t const* data, glpp::TextureLayout const& layout, bool need_to_flip_y)
{
    _size           = size;
    _need_to_flip_y = need_to_flip_y;
    _tex.upload_data(
        img_to_glpp_size(size),
        data,
        layout
    );
    _data_has_been_uploaded = true;
}

void Texture::set_interpolation_mode(glpp::Interpolation interpolation_mode)
{
    bind();
    _tex.set_minification_filter(interpolation_mode);
    _tex.set_magnification_filter(interpolation_mode);
}

void Texture::set_wrap_mode(glpp::Wrap wrap_mode)
{
    bind();
    _tex.set_wrap(wrap_mode);
}

void Texture::bind()
{
    _tex.bind();
}

void Texture::unbind()
{
    glpp::bind_texture<glpp::TextureKind::Tex2D>(0);
}

void Texture::attach_to_slot(GLuint slot) const
{
    if (!_data_has_been_uploaded)
        Log::internal_error("Texture::attach_to_slot", "You must upload some data (at least a width and height) before using the texture.");
    _tex.bind_to_texture_unit(slot);
}

} // namespace Cool::OpenGL

#endif