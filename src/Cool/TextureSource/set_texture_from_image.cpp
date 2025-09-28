#include "set_texture_from_image.h"
#include "easy_ffmpeg/easy_ffmpeg.hpp"

namespace Cool {

void set_texture_from_ffmpeg_image(std::optional<Texture>& texture, ffmpeg::Frame const& image)
{
    // TODO(WebGPU)
    // if (!image.is_different_from_previous_frame && texture.has_value())
    //     return;

    // auto const size = img::Size{
    //     static_cast<unsigned int>(image.width),
    //     static_cast<unsigned int>(image.height),
    // };
    // auto const layout = TextureLayout{
    //     .internal_format = InternalFormat::RGBA,
    //     .channels        = Channels::RGBA,
    //     .texel_data_type = TexelDataType::UnsignedByte,
    // };
    // uint8_t const* const data = image.data;

    // if (!texture)
    //{
    //     texture = Texture{size, data, layout, true /*need_to_flip_y*/};
    // }
    // else
    //{
    //     texture->bind(); // TODO(Video) API texture->while_bound(callback), that automatically rebinds the texture that was previously bound
    //     texture->set_image(size, data, layout, true /*need_to_flip_y*/);
    //     Cool::Texture::unbind();
    // }
}

} // namespace Cool