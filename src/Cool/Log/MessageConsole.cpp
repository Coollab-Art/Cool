#include "MessageConsole.h"
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/markdown.h"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Message.h"
#include "file_logger.hpp"
#include "imgui.h"
#include "spdlog/common.h"
#include "stringify/stringify.hpp"

namespace Cool {

void MessageConsole::send(MessageId& id, Message const& message)
{
    if (!_messages.contains(id.raw()))
    {
        id = _messages.create_shared(internal::MessageWithMetadata{
            .message = message,
        });
    }
    else
    {
        _messages.with_mutable_ref(id.raw(), [&](internal::MessageWithMetadata& data) {
            data.message = message;
            data.count++;
            data.timestamp           = std::chrono::system_clock::now();
            data.monotonic_timestamp = std::chrono::steady_clock::now();
        });
    }

    on_message_sent(id.raw(), message);
}

void MessageConsole::send(Message const& message)
{
    auto const id = _messages.create_raw(internal::MessageWithMetadata{
        .message                = message,
        .forced_to_be_clearable = true,
    });

    on_message_sent(id, message);
}

static auto is_clearable(internal::MessageWithMetadata const& msg) -> bool
{
    return msg.forced_to_be_clearable
           || msg.message.type != MessageType::Error;
}

static auto to_spdlog_level(MessageType type) -> spdlog::level::level_enum
{
    switch (type)
    {
    case MessageType::Info:
        return spdlog::level::info;
    case MessageType::Warning:
        return spdlog::level::warn;
    case MessageType::Error:
        return spdlog::level::err;
    }
    assert(false);
    return spdlog::level::warn;
}

void MessageConsole::on_message_sent(internal::RawMessageId const& id, Message const& message)
{
    _is_open           = true;
    _message_just_sent = id;

    file_logger().log(to_spdlog_level(message.type), fmt::format("[{}] {}", message.title, message.content));
    file_logger().flush(); // We flush as soon as we log a message, to make sure that if the app crashes we won't lose any logs that haven't been flushed yet
}

void MessageConsole::remove(MessageId const& id)
{
    remove(id.raw());
}

void MessageConsole::remove(internal::RawMessageId const& id)
{
    if (!_messages.contains(id))
        return;

    _messages.destroy(id);
    close_window_if_empty();
}

void MessageConsole::close_window_if_empty()
{
    if (_messages.is_empty())
        close_window();
}

void MessageConsole::clear()
{
    clear_if([](auto&&) { return true; });
}

void MessageConsole::clear(MessageType type)
{
    clear_if([&](auto&& message) { return message.type == type; });
}

auto MessageConsole::should_show(internal::MessageWithMetadata const& msg) const -> bool
{
    return !_is_type_hidden.get(msg.message.type);
}

void MessageConsole::clear_if(std::function<bool(const Message&)> const& predicate)
{
    {
        std::unique_lock lock{_messages.mutex()};
        std::erase_if(_messages.underlying_container().underlying_container(), [&](auto&& pair) {
            const auto& message = pair.second;
            return is_clearable(message) && predicate(message.message);
        });
    }

    close_window_if_empty(); // We have only cleared the clearable messages, so it is not guaranteed that the window will be empty.
}

void MessageConsole::close_window()
{
    _is_open          = false;
    _selected_message = {};
}

auto MessageConsole::should_highlight(MessageId const& id) -> bool
{
    return !id.raw().underlying_uuid().is_nil()
           && id.raw() == _selected_message;
}

static auto color(MessageType type) -> ImVec4
{
    switch (type)
    {
    case MessageType::Info:
        return ImGuiNotify::get_style().color_success;
    case MessageType::Warning:
        return ImGuiNotify::get_style().color_warning;
    case MessageType::Error:
        return ImGuiNotify::get_style().color_error;
    }
    assert(false);
    return ImVec4{0, 0, 0, 0};
}

static auto fade_out_if(bool condition, ImVec4 color) -> ImVec4
{
    if (condition)
        color = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

    return color;
}

static auto to_string(MessageType type) -> std::string
{
    switch (type)
    {
    case MessageType::Info:
        return "info";
    case MessageType::Warning:
        return "warning";
    case MessageType::Error:
        return "error";
    }
    assert(false);
    return "";
}

static constexpr auto reason_for_disabling_clear_button = "You can't clear these messages. You have to fix the corresponding errors."; // This is not a good reason_to_disable message when there are no messages, but the console will be hidden anyway.

void MessageConsole::show_number_of_messages_of_given_type(MessageType type)
{
    const auto count = _counts_per_type.get(type);
    if (count > 0)
    {
        ImGui::SameLine();
        ImGui::TextColored(
            fade_out_if(
                _is_type_hidden.get(type),
                color(type)
            ),
            "%zu %s%s",
            count,
            to_string(type).c_str(),
            count > 1 ? "s" : ""
        );
        if (ImGui::IsItemClicked())
            _is_type_hidden.toggle(type);
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::BeginPopupContextItem(to_string(type).c_str()))
        {
            ImGuiExtras::disabled_if(!there_are_clearable_messages(type), reason_for_disabling_clear_button, [&] {
                if (ImGui::Button("Clear"))
                    clear(type);
            });

            ImGui::EndPopup();
        }
    }
}

void MessageConsole::remove_messages_to_keep_size_below(size_t max_number_of_messages)
{
    std::unique_lock lock{_messages.mutex()};
    while (_messages.underlying_container().underlying_container().size() > max_number_of_messages)
    {
        auto const it = std::find_if(_messages.begin(), _messages.end(), [](auto const& pair) {
            return is_clearable(pair.second);
        });
        if (it != _messages.end())
            _messages.underlying_container().underlying_container().erase(it);
        else
            break;
    }
}

void MessageConsole::imgui_window()
{
    remove_messages_to_keep_size_below(99); // If there are too many messages the console starts to cause lag.

    if (_is_open)
    {
        refresh_counts_per_type();

        if (!_message_just_sent.underlying_uuid().is_nil())
        {
            ImGui::SetNextWindowToFront();
        }
        ImGui::Begin(_name.c_str(), nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

        // Menu bar
        ImGuiExtras::background(
            ImGuiNotify::get_style().color_title_background,
            [&]() { imgui_menu_bar(); }
        );
        ImGui::Separator();

        // All the messages
        ImGui::BeginChild(
            "##Messages", ImVec2(0.f, 0.f), false,
            ImGuiWindowFlags_AlwaysUseWindowPadding
        );
        ImGui::PushFont(Font::monospace());
        imgui_show_all_messages();
        ImGui::PopFont();
        ImGui::EndChild();

        ImGui::End();
    }
    _message_just_sent = {};
}

auto MessageConsole::there_are_clearable_messages() const -> bool
{
    std::shared_lock lock{_messages.mutex()};
    return std::any_of(_messages.begin(), _messages.end(), [](auto&& pair) {
        const auto& message = pair.second;
        return is_clearable(message);
    });
}

auto MessageConsole::there_are_clearable_messages(MessageType type) const -> bool
{
    std::shared_lock lock{_messages.mutex()};
    return std::any_of(_messages.begin(), _messages.end(), [&](auto&& pair) {
        const auto& message = pair.second;
        return message.message.type == type
               && is_clearable(message);
    });
}

void MessageConsole::imgui_menu_bar()
{
    ImGuiExtras::disabled_if(!there_are_clearable_messages(), reason_for_disabling_clear_button, [&] {
        if (ImGui::Button("Clear"))
            clear();
    });

    show_number_of_messages_of_given_type(MessageType::Error);
    show_number_of_messages_of_given_type(MessageType::Warning);
    show_number_of_messages_of_given_type(MessageType::Info);
}

static void imgui_button_copy_to_clipboard(ClipboardContent const& clipboard)
{
    if (ImGui::Button(fmt::format("Copy {} to clipboard", clipboard.title).c_str()))
    {
        ImGui::SetClipboardText(clipboard.content.c_str());
        ImGui::CloseCurrentPopup();
    }
};

void MessageConsole::imgui_show_all_messages()
{
    const auto previously_selected_message{_selected_message};
    _selected_message = {};                // close the selected message. And let the following loop set it again if necessary.
    internal::RawMessageId msg_to_clear{}; // Let the loop store a `msg_to_clear`. We don't close the message immediately because it would mess up our for-loop and cause a deadlock with the `lock`.
    {
        std::shared_lock lock{_messages.mutex()};
        for (const auto& id_and_message : _messages)
        {
            const auto& id  = id_and_message.first;
            const auto& msg = id_and_message.second;

            if (!should_show(msg))
                continue;

            const auto widget = [&]() {
                ImGui::PushID(&id);
                ImGui::BeginGroup();

                ImGui::TextColored(
                    color(msg.message.type),
                    "[%s] [#%u] [%s]",
                    Cool::stringify(msg.timestamp).c_str(),
                    msg.count,
                    msg.message.title.c_str()
                );
                ImGui::SameLine();
                ImGuiExtras::markdown(msg.message.content);

                const bool close_button_is_hovered = [&] {
                    if (!is_clearable(msg))
                        return false;

                    ImGui::SameLine();
                    if (ImGuiExtras::close_button())
                    {
                        msg_to_clear = id;
                    }
                    ImGui::SetItemTooltip("%s", ("Close this " + to_string(msg.message.type)).c_str());
                    return ImGui::IsItemHovered();
                }();

                ImGui::SameLine();                                     // Add a dummy to make sure the hitbox of the message
                ImGui::Dummy({ImGui::GetContentRegionAvail().x, 0.f}); // goes till the end of the line (allows hovering to be more intuitive)
                ImGui::EndGroup();

                if (ImGui::IsItemHovered()
                    && !close_button_is_hovered
                    && (ImGui::IsMouseClicked(ImGuiMouseButton_Right)
                        || ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
                {
                    ImGui::OpenPopup("##ContextMenu");
                }
                if (ImGui::IsPopupOpen("##ContextMenu"))
                {
                    ImGui::BeginPopup("##ContextMenu");

                    if (msg.message.clipboard_contents.empty())
                    {
                        imgui_button_copy_to_clipboard({.title = "full message", .content = msg.message.content});
                    }
                    else
                    {
                        for (auto const& clipboard_content : msg.message.clipboard_contents)
                            imgui_button_copy_to_clipboard(clipboard_content);
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopID();

                if (ImGui::IsItemHovered())
                {
                    _selected_message = id;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }
                if (_message_just_sent == id)
                {
                    ImGui::SetScrollHereY(0.5f);
                }
            };

            // Draw highlight of recent messages
            auto const dt = std::chrono::duration<float>{std::chrono::steady_clock::now() - msg.monotonic_timestamp};

            static constexpr float highlight_duration = 0.5f;
            if (dt.count() < highlight_duration)
            {
                ImGuiExtras::highlight(
                    widget,
                    1.f - dt.count() / highlight_duration
                );
            }
            else if (id == previously_selected_message)
            {
                ImGuiExtras::highlight(
                    widget,
                    0.5f
                );
            }
            else
            {
                widget();
            }
        }
    }
    remove(msg_to_clear); // Must clear after the `lock` is gone, otherwise we will deadlock.
}

void MessageConsole::refresh_counts_per_type()
{
    _counts_per_type.reset_to_zero();
    {
        std::shared_lock lock{_messages.mutex()};
        for (const auto& id_and_message : _messages)
            _counts_per_type.increment(id_and_message.second.message.type);
    }
    close_window_if_empty();
}

MessageConsole::MessagesCountPerType::MessagesCountPerType()
{
    reset_to_zero();
}

void MessageConsole::MessagesCountPerType::increment(MessageType type)
{
    _counts_per_type.at(static_cast<size_t>(type))++;
}

void MessageConsole::MessagesCountPerType::reset_to_zero()
{
    _counts_per_type.fill(0);
}

auto MessageConsole::MessagesCountPerType::get(MessageType type) const -> size_t
{
    return _counts_per_type.at(static_cast<size_t>(type));
}

auto MessageConsole::IsTypeHidden::get(MessageType type) const -> bool
{
    return _is_hidden.at(static_cast<size_t>(type));
}

void MessageConsole::IsTypeHidden::set(MessageType type, bool new_value)
{
    _is_hidden.at(static_cast<size_t>(type)) = new_value;
}

void MessageConsole::IsTypeHidden::toggle(MessageType type)
{
    set(type, !get(type));
}

} // namespace Cool
