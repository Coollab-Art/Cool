#include "WindowManager.h"
#include "set_main_window_fullscreen.hpp"

namespace Cool {

auto WindowManager::find(GLFWwindow* glfw_window) -> Window&
{
    return *std::find_if(std::begin(_windows), std::end(_windows), [&](Window const& window) {
        return window.glfw() == glfw_window;
    });
}

void WindowManager::set_main_window(Window& window)
{
    assert(_main_window == nullptr && "You can only have one main window!"); // NOLINT(readability-simplify-boolean-expr)
    _main_window             = &window;
    _global_set_fullscreen() = [&](bool on) {
        _main_window->set_fullscreen(on);
    };
    _global_toggle_fullscreen() = [&]() {
        _main_window->toggle_fullscreen();
    };
}

} // namespace Cool