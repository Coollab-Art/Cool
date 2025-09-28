#pragma once
#include "Cool/Gpu/RenderTarget.h"
#include "Cool/Time/Time.hpp"

namespace Cool {

struct Polaroid {
    // The texture that we will render to
    std::function<TextureRef()> texture;
    // The function that renders the desired image
    std::function<void(img::Size size, Time time, Time delta_time)> render;
};

} // namespace Cool