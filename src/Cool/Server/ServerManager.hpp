#pragma once
#include "Cool/ImGui/ImGuiWindow.h"
#include "serv/serv.hpp"

namespace Cool {

class ServerManager {
public:
    ServerManager();

    void open_config_window() { _window.open(); }
    void imgui_window();

    void set_port(int port);

private:
    std::optional<serv::Server> _server{};
    int                         _port{-1};
    Cool::ImGuiWindow           _window;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            ser20::make_nvp("Port", _port),
            ser20::make_nvp("Window", _window)
        );
    }
    template<class Archive>
    void load(Archive& archive)
    {
        archive(
            _port,
            _window
        );
        set_port(_port); // Open the server
    }
};

inline auto server_manager() -> ServerManager&
{
    static auto instance = ServerManager{};
    return instance;
}

} // namespace Cool