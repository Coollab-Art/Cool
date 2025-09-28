#pragma once
#include <img/img.hpp>
#include "Cool/File/PathChecks.hpp"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Polaroid.hpp"

namespace Cool::ExporterU {

void export_image_using_a_task(img::Size size, Time time, Time delta_time, Polaroid const& polaroid, std::filesystem::path const& file_path);
auto user_accepted_to_ignore_warnings(std::filesystem::path const& file_path, PathChecks const& path_checks) -> bool;

[[nodiscard]] auto notification_after_video_export_success(std::filesystem::path const& path) -> ImGuiNotify::Notification;
[[nodiscard]] auto notification_after_video_export_failure(std::string const& error_message) -> ImGuiNotify::Notification;
[[nodiscard]] auto notification_after_video_export_canceled(std::filesystem::path const& path) -> ImGuiNotify::Notification;

} // namespace Cool::ExporterU
