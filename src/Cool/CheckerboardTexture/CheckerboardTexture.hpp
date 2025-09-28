#pragma once
#include "Cool/Gpu/RenderTarget.h"

namespace Cool {

class CheckerboardTexture {
public:
    auto get(img::Size size) -> RenderTarget const&;

private:
    RenderTarget _render_target{};
};

} // namespace Cool