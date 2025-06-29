#include "Task_WebsocketConnection.hpp"
#include <mutex>
#include "EventQueue.hpp"

namespace Cool {

// TODO(Websocket) Crash when two sockets are connected at the same time ? Is it because they connect to the same address?

// TODO(Websocket) sockpp doesn't actually implement the websocket protocol? Just TCP socket? Is it a problem? Do we need the whole websocket handshake or are we fine with our solution and sending \0 ?

void Task_WebsocketConnection::handle_command(std::string const& command)
{
    command_handler()(command);
}

auto Task_WebsocketConnection::execute() -> TaskCoroutine
{
    std::string            result;
    char                   buf[6]; // TODO(Websocket) What is a good buffer size ?
    sockpp::result<size_t> n;

    while (_socket.is_open())
    {
        // char                   buf[512];
        // sockpp::result<size_t> res;

        // sock.set_non_blocking();
        while (true)
        {
            try
            {
                if (!_socket.is_open())
                    break;
                n = _socket.recv(buf, sizeof(buf));
                if (n.is_error() || n.value() == 0)
                    break;
                for (int i = 0; i < n.value(); ++i)
                {
                    if (buf[i] == 0)
                    {
                        handle_command(result);
                        // std::cout << "Received: " << result << std::endl;
                        result = "";
                    }
                    else
                        result += buf[i];
                }
                // if (n.value() < sizeof(buf))
                //     break;
            }
            catch (...)
            {
                std::cout << "sdf\n";
            }
        }

        {
            auto lock = std::unique_lock{event_queue_mutex()};
            for (auto const& event : event_queue())
            {
                // TODO(Commands) Should use nlohmann json to convert to proper json string, instead of doing all the quoting and escaping of \ in paths etc
                _socket.send(fmt::format(R"JSON({{
                    "event": "ImageExportFinished",
                    "width": {},
                    "height": {},
                    "path": "{}"
                }})JSON",
                                         event.size.width(), event.size.height(), "yo" /* event.path.string() */));
                _socket.write("\0", 1); // Delimiter to tell the python script that the message is finished
            }
            event_queue().clear();
        }

        // _socket.send("TEST Pthonsdf");
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