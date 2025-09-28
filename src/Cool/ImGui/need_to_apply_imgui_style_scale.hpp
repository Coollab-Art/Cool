#pragma once

namespace Cool {

inline auto need_to_apply_imgui_style_scale() -> bool&
{
    static auto instance = true;
    return instance;
}

} // namespace Cool