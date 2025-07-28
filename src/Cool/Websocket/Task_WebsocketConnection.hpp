#pragma once
// #include "Cool/Task/Task.hpp"
// #include "sockpp/tcp_socket.h"

namespace Cool {

// class Task_WebsocketConnection : public Task {
// public:
//     explicit Task_WebsocketConnection(sockpp::tcp_socket socket)
//         : Task{fmt::format("Handling connection with {}", socket.peer_address().to_string())}
//         , _socket{std::move(socket)}
//     {}

// private:
//     auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return false; }
//     auto execute() -> TaskCoroutine override;
//     void cleanup_impl(bool has_been_canceled) override;

//     void handle_command(std::string const& command);

// private:
//     sockpp::tcp_socket _socket;
// };

inline auto request_handler() -> auto& // TODO(Websocket) Find a proper way to declare some commands in Coollab and handle them in Cool. i.e. rethink the commands system
{
    static auto instance = std::function<void(std::string const&)>{};
    return instance;
}

} // namespace Cool