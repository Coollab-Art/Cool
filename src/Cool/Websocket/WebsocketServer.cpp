#include "WebsocketServer.hpp"

namespace Cool {

void run_echo(sockpp::tcp_socket sock)
{
    // char                   buf[512];
    // sockpp::result<size_t> res;

    std::string            result;
    char                   buf[6];
    sockpp::result<size_t> n;

    // sock.set_non_blocking();
    while (true)
    {
        try
        {
            if (!sock.is_open())
                break;
            n = sock.recv(buf, sizeof(buf));
            if (n.is_error())
                break;
            result.append(buf, n.value());
            // if (n.value() < sizeof(buf))
            //     break;
        }
        catch (...)
        {
            std::cout << "sdf\n";
        }
    }

    std::cout << "Received: " << result << std::endl;

    sock.send("TEST Pthonsdf");

    // while ((res = sock.read(buf, sizeof(buf))) && res.value() > 0)
    //     sock.write_n(buf, res.value());

    std::cout
        << "Connection closed from " << sock.peer_address() << std::endl;
}

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
        sockpp::tcp_socket sock = res.release();

        // Create a thread and transfer the new stream to it.
        std::thread thr(run_echo, std::move(sock));
        thr.detach();
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