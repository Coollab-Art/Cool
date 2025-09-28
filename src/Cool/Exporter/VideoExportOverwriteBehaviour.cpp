#include "VideoExportOverwriteBehaviour.h"
#include "Cool/ImGui/ImGuiExtras.h"

namespace Cool {

auto imgui_widget(VideoExportOverwriteBehaviour& behaviour) -> bool
{
    bool const b = ImGui::Combo(
        "Overwrite mode",
        reinterpret_cast<int*>(&behaviour), // NOLINT(*reinterpret-cast)
        "Ask before creating a new folder\0"
        "Ask before overwriting the previous frames\0"
        "Always create a new folder\0"
        "Always overwrite the previous frames\0"
        "\0"
    );
    ImGuiExtras::help_marker(
        R"STR(What happens when trying to export in a folder that already contains some frames.
You can either:
- Create a new empty folder and export your frames there
- Reuse the same folder, in which case the existing frames will get overwritten as new frames get exported)STR"
    );
    return b;
}

} // namespace Cool