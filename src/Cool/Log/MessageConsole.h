#pragma once

#include <reg/reg.hpp>
#include "Message.h"
#include "MessageId.h"

namespace Cool {

namespace internal {
/// Like MessageId, but does not own any message: the message is not destroyed when RawMessageId is destroyed.
using RawMessageId = reg::Id<internal::MessageWithMetadata>;
} // namespace internal

/// Thread-safe
class MessageConsole {
public:
    explicit MessageConsole(std::string name = "Console")
        : _name{std::move(name)}
    {}

    /// If `id` is null, creates a new messages and sets `id` to reference it,
    /// otherwise refreshes the message referenced by `id`.
    void send(MessageId&, Message const&);

    /// Sends a message to the console. No `id` is returned. The message can only be cleared by the user.
    /// Such a message will always be clearable by the user.
    void send(Message const&);

    /// Removes the message, even if it is not supposed to be closable.
    void remove(MessageId const&);

    /// Removes all the closable messages.
    void clear();

    /// Removes all the closable messages of the given type.
    void clear(MessageType);

    /// Removes all the closable messages that verify the `predicate`.
    void clear_if(std::function<bool(const Message&)> const& predicate);

    /// Returns true iff the message is currently selected / hovered by the user.
    /// Allows us to focus the corresponding window / highlight the corresponding part of the UI that requires attention.
    auto should_highlight(MessageId const&) -> bool;

    /// Draws the imgui window with all the messages.
    void imgui_window();

private:
    void remove(internal::RawMessageId const&);
    void close_window_if_empty();
    void close_window();
    void on_message_sent(internal::RawMessageId const&, Message const&);
    void show_number_of_messages_of_given_type(MessageType);
    void refresh_counts_per_type();
    void imgui_menu_bar();
    void imgui_show_all_messages();
    auto there_are_clearable_messages() const -> bool;
    auto there_are_clearable_messages(MessageType) const -> bool;
    auto should_show(internal::MessageWithMetadata const&) const -> bool;
    void remove_messages_to_keep_size_below(size_t max_number_of_messages);

    class MessagesCountPerType {
    public:
        MessagesCountPerType();
        void increment(MessageType);
        void reset_to_zero();
        auto get(MessageType) const -> size_t;

    private:
        std::array<size_t, 3> _counts_per_type{};
    };

    class IsTypeHidden {
    public:
        auto get(MessageType) const -> bool;
        void set(MessageType, bool);
        void toggle(MessageType);

    private:
        std::array<bool, 3> _is_hidden{false};
    };

private:
    reg::OrderedRegistry<internal::MessageWithMetadata> _messages;
    internal::RawMessageId                              _selected_message;
    bool                                                _is_open{false};
    internal::RawMessageId                              _message_just_sent{};
    std::string                                         _name;
    MessagesCountPerType                                _counts_per_type{};
    IsTypeHidden                                        _is_type_hidden{};
};

} // namespace Cool