#pragma once
#include <Cool/ImGui/ImGuiExtras.h>

namespace Cool::internal {

inline auto color_imgui_flags(bool is_hdr) -> ImGuiColorEditFlags
{
    return (is_hdr ? ImGuiColorEditFlags_HDR : 0)
           | ImGuiColorEditFlags_PickerHueWheel
           | ImGuiColorEditFlags_AlphaBar;
}

inline auto color_metadata_widget(bool& is_hdr) -> bool
{
    const bool b = ImGuiExtras::toggle("HDR", &is_hdr);
    ImGuiExtras::help_marker("Allows the RGB values to go outside of the [0, 255] range.");
    return b;
}

inline auto color_and_alpha_metadata_widget(bool& is_hdr) -> bool
{
    const bool b = ImGuiExtras::toggle("HDR", &is_hdr);
    ImGuiExtras::help_marker("Allows the RGBA values to go outside of the [0, 255] range.");
    return b;
}

} // namespace Cool::internal