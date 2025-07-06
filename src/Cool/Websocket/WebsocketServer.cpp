#include "WebsocketServer.hpp"
#include "Cool/Task/TaskManager.hpp"
#include "Cool/Websocket/EventQueue.hpp"
#include "Task_WebsocketConnection.hpp"

namespace Cool {

void WebsocketServer::check_accept_connection()
{
    server(); // HACK make sure the server is created

    // server().poll();     // Non-blocking
    // server().dispatch(); // Process events
    {
        auto lock = std::unique_lock{event_queue_mutex()};
        for (auto const& event : event_queue())
        {
            auto const json = std::visit([](auto&& event) { return event.to_json(); }, event);
            for (auto const& client : server().getClients())
                client->send(json);
        }
        event_queue().clear();
    }
}

auto WebsocketServer::server() -> ix::WebSocketServer&
{
    if (_server.has_value())
        return *_server;

    _server.emplace(12345, "0.0.0.0"); // TODO(Websocket) Choose a more unique port / let users choose the port

    _server->setOnConnectionCallback([](std::weak_ptr<ix::WebSocket>         connection,
                                        std::shared_ptr<ix::ConnectionState> state) {
        std::cout << "Received a connection request from " << state << std::endl;
        auto const co = connection.lock();
        if (!co)
            return;
        // co->send("HELLO");
        co->setOnMessageCallback([connection](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message)
            {
                std::cout << "Received: " << msg->str << std::endl;
                auto const co = connection.lock();
                if (!co)
                    return;

                command_handler()(msg->str);
            }
        });
    });

    if (!_server->listen().first)
    {
        std::cerr << "Server failed to listen" << std::endl;
        // TODO(Websocket) return nullopt then
        // return 1;
    }

    _server->start(); // Starts accepting connections, but doesn't start a poll thread

    return *_server;
}

} // namespace Cool