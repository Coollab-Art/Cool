#pragma once
#include "Cool/WebGPU/Texture.h"

namespace Cool {

auto dummy_image() -> img::Image const&;
auto dummy_texture() -> Texture const&;
auto transparent_texture() -> Texture const&;

} // namespace Cool