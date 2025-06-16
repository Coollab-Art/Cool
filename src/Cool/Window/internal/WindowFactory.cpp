#include "WindowFactory.h"
#include <stdexcept>
#include "Cool/Log/Log.hpp"
#include "Cool/Path/Path.h"
#include "Cool/Core/BuildMode.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"

namespace Cool {

static void glfw_error_callback(int error, const char* description)
{
    Log::internal_error("glfw", fmt::format("Error {}: {}", error, description));
}

static void initialize_glfw()
{
    glfwSetErrorCallback(&glfw_error_callback);
    if (!glfwInit())
    {
        const char* error_description; // NOLINT(*-init-variables)
        glfwGetError(&error_description);
        throw std::runtime_error{fmt::format("GLFW initialization failed:\n{}", error_description)};
    }
}

static void set_imgui_ini_filepath()
{
    static std::string const path = (Cool::Path::user_data() / "imgui.ini").string(); // Needs to be static to keep the char* passed to `IniFilename` alive
    ImGui::GetIO().IniFilename    = path.c_str();
}

static void initialize_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    set_imgui_ini_filepath();
    // Load default settings first. All the new windows that are not in UserData's imgui.ini will get their values from here
    ImGui::LoadIniSettingsFromDisk((Cool::Path::default_user_data() / "imgui.ini").string().c_str());
    // Load user-defined settings second. They will override all the windows that were already known by the user, and which they might have customized
    ImGui::LoadIniSettingsFromDisk((Cool::Path::user_data() / "imgui.ini").string().c_str());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingAlwaysTabBar = false;
    io.ConfigDebugHighlightIdConflicts = is_building_as(BuildMode::Debug);
#if !defined(COOL_UPDATE_APP_ON_SEPARATE_THREAD)        // Platform windows freeze if we are not rendering on the main thread (TODO(JF) : need to investigate that bug ; it is probably coming directly from ImGui)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Must be done here at creation of the App, otherwise we can't toggle it at runtime.
#endif
}

WindowFactory::WindowFactory()
{
    initialize_glfw();
    initialize_imgui();
}

WindowFactory::~WindowFactory()
{
    WindowFactory_Impl::shut_down(_window_manager);
}

auto WindowFactory::make_main_window(WindowConfig const& config) -> Window&
{
    auto& window = WindowFactory_Impl::make_window(config, _window_manager);
    WindowFactory_Impl::setup_main_window(window);
    _window_manager.set_main_window(window);
    return window;
}

auto WindowFactory::make_secondary_window(const WindowConfig& config) -> Window&
{
    auto& window = WindowFactory_Impl::make_window(config, _window_manager);
    WindowFactory_Impl::setup_secondary_window(window, _window_manager);
    return window;
}

} // namespace Cool