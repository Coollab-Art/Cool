#pragma once
#include "ixwebsocket/IXWebSocketServer.h"

namespace Cool {

class WebsocketServer {
public:
    void check_accept_connection();

private:
    auto server() -> ix::WebSocketServer&;

private:
    std::optional<ix::WebSocketServer> _server;
};

inline auto websocket_server() -> WebsocketServer&
{
    static auto instance = WebsocketServer{};
    return instance;
}

} // namespace Cool