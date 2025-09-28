#include "ExporterU.h"
#include <memory>
#include <open/open.hpp>
#include "Cool/File/File.h"
#include "Cool/Image/SaveImage.h"
#include "Cool/Log/boxer_show.hpp"
#include "Cool/Task/Task.hpp"
#include "Cool/Task/TaskManager.hpp"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Task_SaveImage.hpp"

namespace Cool::ExporterU {

void export_image_using_a_task(img::Size size, Time time, Time delta_time, Polaroid const& polaroid, std::filesystem::path const& file_path)
{
    polaroid.render(size, time, delta_time);
    task_manager().submit(std::make_shared<Task_SaveImage>(file_path, polaroid.texture().download_pixels()));
}

auto user_accepted_to_ignore_warnings(std::filesystem::path const& file_path, PathChecks const& path_checks) -> bool
{
    auto const warnings = path_checks.compute_all_warnings(file_path);
    if (warnings.empty())
        return true;

    auto message = ""s;
    for (auto const& warning : warnings)
    {
        if (!message.empty())
            message += '\n';
        message += "- ";
        message += warning;
    }

    return boxer_show(message.c_str(), "Ignore warnings?", boxer::Style::Warning, boxer::Buttons::OKCancel)
           == boxer::Selection::OK;
}

auto notification_after_video_export_success(std::filesystem::path const& path) -> ImGuiNotify::Notification
{
    return {
        .type                 = ImGuiNotify::Type::Success,
        .title                = "Video Export Success",
        .custom_imgui_content = [path]() {
            if (ImGui::Button(fmt::format("Open \"{}\" folder", Cool::File::file_name(path)).c_str()))
                open_folder_in_explorer(path);
        },
    };
}

auto notification_after_video_export_failure(std::string const& error_message) -> ImGuiNotify::Notification
{
    return {
        .type    = ImGuiNotify::Type::Error,
        .title   = "Video Export Failed",
        .content = error_message,
    };
}

auto notification_after_video_export_canceled(std::filesystem::path const& path) -> ImGuiNotify::Notification
{
    return {
        .type                 = ImGuiNotify::Type::Warning,
        .title                = "Video Export Canceled",
        .custom_imgui_content = [path]() {
            if (ImGui::Button(fmt::format("Open \"{}\" folder", Cool::File::file_name(path)).c_str()))
                open_folder_in_explorer(path);
        },
    };
}

} // namespace Cool::ExporterU
