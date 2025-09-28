#pragma once
#include "MessageConsole.h"
#include "MessageId.h"
#include "message_console.hpp"

namespace Cool {

class TestMessageConsole {
public:
    TestMessageConsole();

    void imgui();

private:
    struct Message {
        Cool::MessageType type{Cool::MessageType::Error};
        std::string       title{"Test"};
        std::string       content{"Hello!"};
        Cool::MessageId   id{};

        void send()
        {
            message_console().send(
                id,
                {
                    .type    = type,
                    .title   = title,
                    .content = content,
                }
            );
        }
    };

    void imgui(Message& message);

private:
    std::vector<Message>     _messages{};
    size_t                   _next_message_number{0};
    std::optional<MessageId> _scoped_message_id{};
};

} // namespace Cool
