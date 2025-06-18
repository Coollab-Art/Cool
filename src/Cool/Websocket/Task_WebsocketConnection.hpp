#pragma once
#include "Cool/Task/Task.hpp"
#include "sockpp/tcp_socket.h"

namespace Cool {

class Task_WebsocketConnection : public Task {
public:
    explicit Task_WebsocketConnection(sockpp::tcp_socket socket)
        : Task{fmt::format("Handling connection with {}", socket.peer_address().to_string())}
        , _socket{std::move(socket)}
    {}

private:
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return false; }
    auto execute() -> TaskCoroutine override;
    void cleanup_impl(bool has_been_canceled) override;

private:
    sockpp::tcp_socket _socket;
};

} // namespace Cool