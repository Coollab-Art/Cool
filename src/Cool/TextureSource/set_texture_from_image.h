#pragma once
#include "Cool/WebGPU/Texture.h"

namespace ffmpeg {
struct Frame;
}

namespace Cool {

void set_texture_from_ffmpeg_image(std::optional<Texture>& texture, ffmpeg::Frame const& image);

} // namespace Cool