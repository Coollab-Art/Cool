#pragma once
#include "Cool/Gpu/RenderTarget.h"

namespace Cool {

class CheckerboardTexture {
public:
    auto get(img::Size size) -> RenderTarget const&;

private:
    RenderTarget _render_target{TextureFormat{.num_components = 3, .type = PixelType::UInt8}};
};

} // namespace Cool