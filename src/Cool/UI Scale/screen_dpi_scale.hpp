#pragma once

namespace Cool {

inline auto screen_dpi_scale() -> float&
{
    static auto instance = 1.f;
    return instance;
}

} // namespace Cool