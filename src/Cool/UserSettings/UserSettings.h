#pragma once
#include <smart/smart.hpp>
#include "Cool/Exporter/VideoExportOverwriteBehaviour.h"
#include "Cool/Serialization/Json.hpp"
#include "Cool/Serialization/JsonAutoSerializer.hpp"
#include "Cool/Time/Time.hpp"
#include "Cool/UI Scale/need_to_rebuild_fonts.hpp"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

auto should_enable_multi_viewport_by_default() -> bool;

struct UserSettings {
    bool                          autosave_enabled{true};
    Time                          autosave_delay{5s};
    bool                          extra_icons{true};
    float                         camera2D_zoom_sensitivity{1.1f};
    bool                          single_click_to_input_in_drag_widgets{true};
    bool                          enable_multi_viewport{should_enable_multi_viewport_by_default()};
    VideoExportOverwriteBehaviour video_export_overwrite_behaviour{VideoExportOverwriteBehaviour::AskBeforeCreatingNewFolder};
    float                         ui_zoom{1.f};

    void change_ui_zoom(float delta);
    void set_ui_zoom(float zoom);
    void apply_ui_zoom() const;
    void apply_ui_zoom_preview() const;
    void apply_multi_viewport_setting() const;

    auto imgui() -> bool;
    auto imgui_autosave() -> bool;
    auto imgui_extra_icons() -> bool;
    auto imgui_camera2D_zoom_sensitivity() -> bool;
    auto imgui_single_click_to_input_in_drag_widgets() -> bool;
    auto imgui_enable_multi_viewport() -> bool;
    auto imgui_video_export_overwrite_behaviour() -> bool;
    auto imgui_ui_zoom() -> bool;

    void update() { _serializer.update(); }
    void save() { _serializer.save(); }

private:
    float                       _ui_scale_at_the_beginning_of_preview{};
    ImGuiNotify::NotificationId _notif_ui_zoom{};

private:
    // Must be declared last, after all the variables it serializes, so that the values it loads overwrite the default values, and not the other way around
    JsonAutoSerializer _serializer{
        "user_settings.json",
        false /*autosave_when_destroyed*/, // This is a static instance, so saving it in the destructor is dangerous because we don't know when it will happen exactly. Instead, we call save manually at the end of the run()
        [&](nlohmann::json const& json) {
            json_get(json, "Autosave enabled", autosave_enabled);
            json_get(json, "Autosave delay", autosave_delay);
            autosave_delay = Time::seconds(smart::keep_above(1., autosave_delay.as_seconds_double())); // Make sure the delay is at least 1 second, to avoid lagging
            json_get(json, "Extra icons", extra_icons);
            json_get(json, "Camera 2D zoom sensitivity", camera2D_zoom_sensitivity);
            json_get(json, "Single click to input in drag widgets", single_click_to_input_in_drag_widgets);
            json_get(json, "Enable multi viewport", enable_multi_viewport);
            apply_multi_viewport_setting();
            json_get(json, "Video export overwrite behaviour", video_export_overwrite_behaviour);
            json_get(json, "UI Scale", ui_zoom); // Backward compatibility
            json_get(json, "UI Zoom", ui_zoom);
            need_to_rebuild_fonts() = true;
        },
        [&](nlohmann::json& json) {
            json_set(json, "Autosave enabled", autosave_enabled);
            json_set(json, "Autosave delay", autosave_delay);
            json_set(json, "Extra icons", extra_icons);
            json_set(json, "Camera 2D zoom sensitivity", camera2D_zoom_sensitivity);
            json_set(json, "Single click to input in drag widgets", single_click_to_input_in_drag_widgets);
            json_set(json, "Enable multi viewport", enable_multi_viewport);
            json_set(json, "Video export overwrite behaviour", video_export_overwrite_behaviour);
            json_set(json, "UI Zoom", ui_zoom);
        },
        true /*use_shared_user_data*/
    };
};

inline auto user_settings() -> UserSettings&
{
    static UserSettings instance;
    return instance;
}

} // namespace Cool
