#include "Task_WebsocketConnection.hpp"

namespace Cool {

// TODO(Websocket) Crash when two sockets are connected at the same time ? Is it because they connect to the same address?

auto Task_WebsocketConnection::execute() -> TaskCoroutine
{
    while (true)
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
                if (!_socket.is_open())
                    break;
                n = _socket.recv(buf, sizeof(buf));
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

        _socket.send("TEST Pthonsdf");
        co_await SuspendTask{}; // TODO(Websocket) should sleep for 100ms
    }
    // while ((res = sock.read(buf, sizeof(buf))) && res.value() > 0)
    //     sock.write_n(buf, res.value());
}

void Task_WebsocketConnection::cleanup_impl(bool /* has_been_canceled */)
{
    std::cout << "Connection closed from " << _socket.peer_address() << std::endl;
}

} // namespace Cool