#pragma once
#include <img/img.hpp>
#include "Cool/File/PathChecks.hpp"
#include "Cool/Websocket/Event.hpp"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Polaroid.hpp"

namespace Cool::ExporterU {

void export_image_using_a_task(img::Size size, Time time, Time delta_time, Polaroid const& polaroid, std::filesystem::path const& file_path, std::function<void(Cool::Event)> const& start_callback = nullptr, std::function<void(Cool::Event)> const& end_callback = nullptr);
auto user_accepted_to_ignore_warnings(std::filesystem::path const& file_path, PathChecks const& path_checks) -> bool;

[[nodiscard]] auto notification_after_video_export_success(std::filesystem::path const& path) -> ImGuiNotify::Notification;
[[nodiscard]] auto notification_after_video_export_failure(std::filesystem::path const& path, std::string const& error_message) -> ImGuiNotify::Notification;
[[nodiscard]] auto notification_after_video_export_canceled(std::filesystem::path const& path) -> ImGuiNotify::Notification;

} // namespace Cool::ExporterU
