#pragma once
#include "Window.h"

namespace Cool {

class WindowManager {
public:
    [[nodiscard]] auto windows() -> std::list<Window>& { return _windows; }
    [[nodiscard]] auto find(GLFWwindow* glfw_window) -> Window&;

    [[nodiscard]] auto main_window() -> Window&
    {
        assert(_main_window != nullptr && "You forgot to create a main window!"); // NOLINT(readability-simplify-boolean-expr)
        return *_main_window;
    }

private:
    friend class WindowFactory;
    void set_main_window(Window& window);

private:
    std::list<Window> _windows;
    Window*           _main_window = nullptr;
};

} // namespace Cool
