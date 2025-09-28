#pragma once
#include <Cool/View/ViewsManager.h>
#include <GLFW/glfw3.h>
#include "AppManagerConfig.h"
#include "IApp.h"

namespace Cool {

/// Wrapper for an App. It handles the user events, the ImGui UI, and the windows' updates.
class AppManager {
public:
    /**
     * @param app An instance of an App class that you have to implement, deriving from IApp.
     * @param config Configuration options that control the behaviour of the AppManager
     */
    AppManager(ViewsManager& views, IApp& app, AppManagerConfig config);

    /// Runs the app's update loop continuously, until the user closes the window.
    /// Also calls `on_update()` after every update.
    void run(std::function<void()> const& on_update);

    /// Equivalent to the user clicking the close button or pressing ALT+F4
    void close_application();
    /// Does nothing if some tasks that need user confirmation to cancel are still running
    void close_application_if_all_tasks_are_done();

private:
    void update();
    auto should_close_window() const -> bool;
    void restore_imgui_ini_state_ifn();
    void imgui_render(IApp&);
    void imgui_windows();
    /// Returns true iff we can Present
    [[nodiscard]] auto imgui_end_frame() -> bool;

    void request_rerender_thread_safe();

    static void key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);

    void dispatch_all_events();
    void dispatch_mouse_movement();
    void dispatch_mouse_click();
    void dispatch_mouse_scroll();

private:
    ViewsManager&    _views;
    IApp&            _app;
    AppManagerConfig _config;

    int _frames_count{0};

    std::atomic<bool> _wants_to_request_rerender{false};
};

} // namespace Cool