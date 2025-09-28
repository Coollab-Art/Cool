
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/Log/file_logger.hpp"
#include "message_console.hpp"

namespace Cool::Log {

void info(std::string title, std::string content, std::vector<ClipboardContent> clipboard_contents)
{
    message_console().send(Message{
        .type               = MessageType::Info,
        .title              = std::move(title),
        .content            = std::move(content),
        .clipboard_contents = std::move(clipboard_contents),
    });
}

void warning(std::string title, std::string content, std::vector<ClipboardContent> clipboard_contents)
{
    message_console().send(Message{
        .type               = MessageType::Warning,
        .title              = std::move(title),
        .content            = std::move(content),
        .clipboard_contents = std::move(clipboard_contents),
    });
}

void error(std::string title, std::string content, std::vector<ClipboardContent> clipboard_contents)
{
    message_console().send(Message{
        .type               = MessageType::Error,
        .title              = std::move(title),
        .content            = std::move(content),
        .clipboard_contents = std::move(clipboard_contents),
    });
}

void internal_info(std::string title, std::string content, std::vector<ClipboardContent> clipboard_contents, std::optional<bool> should_log_to_user)
{
    if ((should_log_to_user.has_value() && *should_log_to_user) || DebugOptions::log_internal_warnings())
        Log::info(std::move(title), std::move(content), std::move(clipboard_contents));
    else
    {
        file_logger().info(fmt::format("[{}] {}", std::move(title), std::move(content)));
        file_logger().flush(); // We flush as soon as we log a message, to make sure that if the app crashes we won't lose any logs that haven't been flushed yet
    }
}

void internal_warning(std::string title, std::string content, std::vector<ClipboardContent> clipboard_contents, std::optional<bool> should_log_to_user)
{
    if ((should_log_to_user.has_value() && *should_log_to_user) || DebugOptions::log_internal_warnings())
        Log::warning(std::move(title), std::move(content), std::move(clipboard_contents));
    else
    {
        file_logger().warn(fmt::format("[{}] {}", std::move(title), std::move(content)));
        file_logger().flush(); // We flush as soon as we log a message, to make sure that if the app crashes we won't lose any logs that haven't been flushed yet
    }
}

void internal_error(std::string title, std::string content, std::vector<ClipboardContent> clipboard_contents, std::optional<bool> should_log_to_user)
{
    if ((should_log_to_user.has_value() && *should_log_to_user) || DebugOptions::log_internal_warnings())
        Log::error(std::move(title), std::move(content), std::move(clipboard_contents));
    else
    {
        file_logger().error(fmt::format("[{}] {}", std::move(title), std::move(content)));
        file_logger().flush(); // We flush as soon as we log a message, to make sure that if the app crashes we won't lose any logs that haven't been flushed yet
    }
}

} // namespace Cool::Log
