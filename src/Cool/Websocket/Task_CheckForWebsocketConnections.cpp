#include "Task_CheckForWebsocketConnections.hpp"
#include "Cool/Task/TaskManager.hpp"
#include "WebsocketServer.hpp"

namespace Cool {

auto Task_CheckForWebsocketConnections::execute() -> TaskCoroutine
{
    websocket_server().check_accept_connection(); // NB this is not thread safe but its ok because only this task is accessing the websocket server
    task_manager().submit(after(100ms), std::make_shared<Task_CheckForWebsocketConnections>());
    co_return;
}

} // namespace Cool