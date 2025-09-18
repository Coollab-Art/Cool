#pragma once

namespace Cool {

inline auto _global_toggle_fullscreen() -> std::function<void()>&
{
    static auto instance = std::function<void()>{};
    return instance;
}

inline auto _global_set_fullscreen() -> std::function<void(bool)>&
{
    static auto instance = std::function<void(bool)>{};
    return instance;
}

inline void toggle_main_window_fullscreen()
{
    _global_toggle_fullscreen()();
}

inline void set_main_window_fullscreen(bool on)
{
    _global_set_fullscreen()(on);
}

} // namespace Cool