#include "imgui_config.h"
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/Path/Path.h"

namespace Cool {

static void set_imgui_ini_filepath()
{
    static std::string const path = (Cool::Path::user_data() / "imgui.ini").string(); // Needs to be static to keep the char* passed to `IniFilename` alive
    ImGui::GetIO().IniFilename    = path.c_str();
}

static void initialize_imgui()
{
    set_imgui_ini_filepath();
    // Load default settings first. All the new windows that are not in UserData's imgui.ini will get their values from here
    ImGui::LoadIniSettingsFromDisk((Cool::Path::default_user_data() / "imgui.ini").string().c_str());
    // Load user-defined settings second. They will override all the windows that were already known by the user, and which they might have customized
    ImGui::LoadIniSettingsFromDisk((Cool::Path::user_data() / "imgui.ini").string().c_str());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingAlwaysTabBar = true;
//#if !defined(COOL_UPDATE_APP_ON_SEPARATE_THREAD)        // Platform windows freeze if we are not rendering on the main thread (TODO(JF) : need to investigate that bug ; it is probably coming directly from ImGui)
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Must be done here at creation of the App, otherwise we can't toggle it at runtime.
//#endif
}

} // namespace Cool