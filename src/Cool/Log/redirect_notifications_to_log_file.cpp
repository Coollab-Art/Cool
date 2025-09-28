#include "redirect_notifications_to_log_file.hpp"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "file_logger.hpp"

namespace Cool {

static auto to_spdlog_level(ImGuiNotify::Type type)
{
    switch (type)
    {
    case ImGuiNotify::Type::Success:
    case ImGuiNotify::Type::Info:
        return spdlog::level::info;
    case ImGuiNotify::Type::Warning:
        return spdlog::level::warn;
    case ImGuiNotify::Type::Error:
        return spdlog::level::err;
    }
    assert(false);
    return spdlog::level::warn;
}

void redirect_notifications_to_log_file()
{
    ImGuiNotify::add_notification_callback([](ImGuiNotify::Notification const& notification) {
        file_logger().log(to_spdlog_level(notification.type), fmt::format("[{}] {}", notification.title, notification.content));
        file_logger().flush(); // We flush as soon as we log a message, to make sure that if the app crashes we won't lose any logs that haven't been flushed yet
    });
}

} // namespace Cool