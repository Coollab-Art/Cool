#pragma once
#include "sockpp/tcp_acceptor.h"

namespace Cool {

class WebsocketServer {
public:
    void check_accept_connection();

private:
    auto acceptor() -> sockpp::tcp_acceptor&;

private:
    std::optional<sockpp::tcp_acceptor> _acceptor;
};

inline auto websocket_server() -> WebsocketServer&
{
    static auto instance = WebsocketServer{};
    return instance;
}

} // namespace Cool