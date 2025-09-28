#pragma once

namespace Cool {

inline auto need_to_rebuild_fonts() -> bool&
{
    static auto instance = true;
    return instance;
}

} // namespace Cool