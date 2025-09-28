#include "UserSettings.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/need_to_apply_imgui_style_scale.hpp"
#include "Cool/Input/CTRL_OR_CMD.hpp"
#include "Cool/UI Scale/need_to_rebuild_fonts.hpp"
#include "Cool/UI Scale/ui_scale.hpp"
#include "imgui.h"

namespace Cool {

auto UserSettings::imgui() -> bool
{
    bool b = false;

    ImGuiExtras::separator_text("Autosave");
    b |= imgui_autosave();

    ImGuiExtras::separator_text("UI");
    b |= imgui_ui_zoom();
    b |= imgui_extra_icons();

    ImGuiExtras::separator_text("Camera");
    b |= imgui_camera2D_zoom_sensitivity();

    ImGuiExtras::separator_text("Miscellaneous");
    b |= imgui_single_click_to_input_in_drag_widgets();
    b |= imgui_enable_multi_viewport();

    if (b)
        _serializer.save();
    return b;
}

auto UserSettings::imgui_autosave() -> bool
{
    return ImGuiExtras::toggle_with_submenu("Autosave", &autosave_enabled, [&]() {
        bool const b = imgui_drag_time("Delay", &autosave_delay, imgui_drag_time_params{.show_milliseconds = false, .min = 1.});
        if (b)
            autosave_delay = Time::seconds(smart::keep_above(1., autosave_delay.as_seconds_double())); // Make sure the delay is at least 1 second, to avoid lagging
        return b;
    });
}

auto UserSettings::imgui_extra_icons() -> bool
{
    bool const b = ImGuiExtras::toggle("Extra Icons", &extra_icons);
    ImGuiExtras::help_marker("Adds additional icons for some menus, buttons, etc.");
    return b;
}

auto UserSettings::imgui_camera2D_zoom_sensitivity() -> bool
{
    return ImGui::SliderFloat("Camera 2D zoom sensitivity", &camera2D_zoom_sensitivity, 1.001f, 1.2f);
}

auto UserSettings::imgui_single_click_to_input_in_drag_widgets() -> bool
{
    bool const b = ImGuiExtras::toggle("Single-click to input in \"Drag number\" widgets.", &single_click_to_input_in_drag_widgets);
    ImGuiExtras::help_marker("When disabled, you need to double-click or " CTRL_OR_CMD "+click on a \"Drag number\" widget to be able to write down a specific value.");
    return b;
}

auto UserSettings::imgui_enable_multi_viewport() -> bool
{
    bool const b = ImGuiExtras::toggle("Enable multi-viewport", &enable_multi_viewport);
    ImGuiExtras::help_marker(
        "When enabled, allows you to drag windows outside of the main window."
#if defined(__linux__)
        "\nNote that on Linux this can cause issues with context menus if you use a custom window manager."
#endif
    );
    if (b)
        apply_multi_viewport_setting();
    return b;
}

auto UserSettings::imgui_video_export_overwrite_behaviour() -> bool
{
    return imgui_widget(video_export_overwrite_behaviour);
}

static constexpr float min_ui_zoom = 0.5f;
static constexpr float max_ui_zoom = 2.f;

auto UserSettings::imgui_ui_zoom() -> bool
{
    auto const b = ImGui::DragFloat("UI Zoom", &ui_zoom, 0.001f, min_ui_zoom, max_ui_zoom, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemActivated())
        _ui_scale_at_the_beginning_of_preview = Cool::ui_scale();
    if (ImGui::IsItemDeactivatedAfterEdit())
        apply_ui_zoom();
    else if (b)
        apply_ui_zoom_preview();
    return b;
}

void UserSettings::change_ui_zoom(float delta)
{
    static float const zoom_factor = std::pow(max_ui_zoom, 1.f / 4.f); // So that applying it 4 times gives us the max ui zoom
    set_ui_zoom(ui_zoom * std::pow(zoom_factor, delta));
    ImGuiNotify::send_or_change(
        _notif_ui_zoom,
        {
            .type                 = ImGuiNotify::Type::Info,
            .title                = "UI Zoom",
            .custom_imgui_content = [&id = _notif_ui_zoom]() {
                ImGui::TextUnformatted(fmt::format("{:.3f}", Cool::user_settings().ui_zoom).c_str());
                if (ImGui::Button("Reset"))
                {
                    Cool::user_settings().set_ui_zoom(1.f);
                    ImGuiNotify::close_immediately(id);
                }
            },
            .duration = 1s,
        }
    );
}

void UserSettings::set_ui_zoom(float zoom)
{
    ui_zoom = std::clamp(zoom, min_ui_zoom, max_ui_zoom);
    apply_ui_zoom();
}

void UserSettings::apply_ui_zoom() const
{
    need_to_rebuild_fonts()           = true;
    need_to_apply_imgui_style_scale() = true;
}

void UserSettings::apply_ui_zoom_preview() const
{
    ImGui::GetIO().FontGlobalScale    = Cool::ui_scale() / _ui_scale_at_the_beginning_of_preview;
    need_to_apply_imgui_style_scale() = true;
}

void UserSettings::apply_multi_viewport_setting() const
{
    // #if defined(COOL_UPDATE_APP_ON_SEPARATE_THREAD)
    //     return; // Platform windows freeze if we are not rendering on the main thread (TODO(JF) : need to investigate that bug ; it is probably coming directly from ImGui)
    // #endif
    if (enable_multi_viewport)
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    else
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
}

auto should_enable_multi_viewport_by_default() -> bool
{
#if !defined(__linux__)
    return true;
#else
    // On Linux this can conflict with tiling WM and make our context menus behave weirdly.
    // https://cdn.discordapp.com/attachments/848704719987671070/1127711921651597332/ui-linux.mp4
    // https://github.com/ocornut/imgui/issues/2117
    // For example i3 has issues with it.
    // The desktops listed here should be okay though: https://wiki.archlinux.org/title/Xdg-utils#Environment_variables
    char const* var = std::getenv("XDG_CURRENT_DESKTOP");
    if (!var)
        return false;
    auto const vari = std::string{var};
    return vari == "Cinnamon"
           || vari == "X-Cinnamon"
           || vari == "Deepin"
           || vari == "DEEPIN"
           || vari == "deepin"
           || vari == "ENLIGHTENMENT"
           || vari == "GNOME"
           || vari == "GNOME-Flashback"
           || vari == "GNOME-Flashback:GNOME"
           || vari == "KDE"
           || vari == "LXDE"
           || vari == "LXQt"
           || vari == "MATE"
           || vari == "XFCE";
#endif
}

} // namespace Cool
