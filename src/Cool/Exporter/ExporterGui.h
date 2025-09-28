#pragma once
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/ImGui/icon_fmt.h"
#include "ExportSize.h"
#include "VideoExportParams.h"
#include "VideoExportProcess.h"

namespace Cool {

/// Callbacks that will be called when the corresponding buttons are clicked.
/// If nothing is specified, the default behaviour is just to open the window.
/// If you specify your own callback, you have to ask for the window to open yourself, by calling
/// `exporter.image_export_window().open()` or `exporter.video_export_window().open()`.
struct exporter_imgui_menu_items_Params {
    std::optional<std::function<void()>> open_image_exporter = {};
    std::optional<std::function<void()>> open_video_exporter = {};
};

struct exporter_imgui_windows_Params {
    Polaroid  polaroid;
    Time      time;
    Time      delta_time;
    TimeSpeed time_speed;
    /// Note that by the time this function is called, the image will not have been exported yet since this is done in a task
    std::function<void(std::filesystem::path const&)> on_image_export_start                      = [](std::filesystem::path const&) {};
    std::function<void()>                             on_video_export_start                      = []() {};
    std::function<void()>                             widgets_in_window_video_export_in_progress = []() {};
    PathChecks                                        image_path_checks                          = {};
};

class ExporterGui {
public:
    [[nodiscard]] static auto is_exporting(std::optional<VideoExportProcess> const& video_export_process) -> bool { return video_export_process.has_value(); }

    auto image_export_window() -> ImGuiWindow& { return _image_export_window; }
    auto video_export_window() -> ImGuiWindow& { return _video_export_window; }

    /// Displays all the currently active windows.
    void imgui_windows(exporter_imgui_windows_Params const&, std::optional<VideoExportProcess>&);

    /// The buttons to open the different exporter windows.
    void imgui_menu_items(exporter_imgui_menu_items_Params const& = {}, std::optional<std::string> longest_text = {});

    /// Call this after your rendering code. If we are exporting it will export the current frame and decide if the export should continue.
    void update(Polaroid const&, std::optional<VideoExportProcess>&);

    void set_shared_aspect_ratio(SharedAspectRatio& shared_aspect_ratio) { _export_size.set_shared_aspect_ratio(shared_aspect_ratio); }

    auto export_size() const -> img::Size { return _export_size; }
    auto image_export_path() -> std::filesystem::path;

private:
    /// Starts the export of the image sequence. You must then call update() on every frame after your rendering code.
    /// Return true iff it successfully began
    auto begin_video_export(std::optional<VideoExportProcess>&, TimeSpeed time_speed, std::function<void()> const& on_video_export_start) -> bool;
    /// Ends the export of the image sequence. It will be called automatically by update() once the end timestamp is reached. You can also call it yourself to early exit of the export.
    static void        end_video_export(std::optional<VideoExportProcess>&);
    void               imgui_window_export_image(Polaroid polaroid, Time time, Time delta_time, std::function<void(std::filesystem::path const&)> const& on_image_export_start, PathChecks const& path_checks);
    void               imgui_window_export_video(std::function<void()> const& widgets_in_window_video_export_in_progress, std::function<void()> const& on_video_export_start, std::optional<VideoExportProcess>&, TimeSpeed time_speed);
    [[nodiscard]] auto user_accepted_our_frames_overwrite_behaviour() -> bool;

    [[nodiscard]] auto folder_path_for_video() -> std::filesystem::path&;
    [[nodiscard]] auto image_file_path_beeing_edited_in_ui() -> std::filesystem::path&;

private:
    ExportSize                           _export_size{};
    std::optional<std::filesystem::path> _image_file_path_beeing_edited_in_ui{}; // We delay the initialisation to give users time to save their project. If they do so before trying to export we will use the project folder as the default folder. Otherwise we will fall back to the user data folder.
    ImGuiWindow                          _image_export_window{icon_fmt("Export an Image", ICOMOON_IMAGE), ImGuiWindowConfig{.is_modal = true}};

    std::optional<std::filesystem::path> _folder_path_for_video{}; // We delay the initialisation to give users time to save their project. If they do so before trying to export we will use the project folder as the default folder. Otherwise we will fall back to the user data folder.
    ImGuiWindow                          _video_export_window{icon_fmt("Export a Video", ICOMOON_FILM), ImGuiWindowConfig{.is_modal = true}};
    VideoExportParams                    _video_export_params;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Video Settings", _video_export_params),
            ser20::make_nvp("Image Size", _export_size),
            ser20::make_nvp("Image Output File", _image_file_path_beeing_edited_in_ui),
            ser20::make_nvp("Video Output Folder", _folder_path_for_video)
        );
    }
};

} // namespace Cool