#include "WebsocketServer.hpp"
#include "Cool/Task/TaskManager.hpp"
#include "Task_WebsocketConnection.hpp"

namespace Cool {

void WebsocketServer::check_accept_connection()
{
    sockpp::inet_address peer;

    // Accept a new client connection
    acceptor().set_non_blocking();
    if (auto res = acceptor().accept(&peer); !res)
    {
        // TODO(Websocket) error handling
        // std::cerr << "Error accepting incoming connection: " << res.error_message() << std::endl;
    }
    else
    {
        std::cout << "Received a connection request from " << peer << std::endl;
        task_manager().submit(std::make_shared<Task_WebsocketConnection>(res.release()));
    }
}

auto WebsocketServer::acceptor() -> sockpp::tcp_acceptor&
{
    if (_acceptor.has_value())
        return *_acceptor;

    sockpp::initialize();

    in_port_t       port = sockpp::TEST_PORT;
    std::error_code ec;
    _acceptor = sockpp::tcp_acceptor{port, 4, ec};

    if (ec)
    {
        std::cerr << "Error creating the acceptor: " << ec.message() << std::endl; // TODO(Websocket) proper errorr handling
        // TODO(Websocket) return nullopt then
        // return 1;
    }

    return *_acceptor;
}

} // namespace Cool