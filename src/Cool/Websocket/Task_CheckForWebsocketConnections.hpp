#pragma once
#include "Cool/Task/Task.hpp"

namespace Cool {

class Task_CheckForWebsocketConnections : public Task {
public:
    Task_CheckForWebsocketConnections()
        : Task{"Checking for incoming websocket connections"}
    {}

private:
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return false; }
    auto execute() -> TaskCoroutine override;
};

} // namespace Cool