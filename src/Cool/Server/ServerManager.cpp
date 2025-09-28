#include "ServerManager.hpp"
#include <imgui.h>
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/OSC/OSCChannel.h"
#include "Cool/OSC/OSCManager.h"

namespace Cool {

ServerManager::ServerManager()
    : _window{
          Cool::icon_fmt("Server", ICOMOON_CONNECTION),
          Cool::ImGuiWindowConfig{
              .is_modal   = false,
              .start_open = false,
          }
      }
{
}

void ServerManager::imgui_window()
{
    _window.show([&](bool /* is_opening */) {
        // Port
        ImGuiExtras::input_port("Port", &_port);
        if (ImGui::IsItemDeactivated()) // Since setting the port is laggy, we only do it once, when the user has finished typing
            set_port(_port);
        // Server state
        ImGui::PushFont(Font::italic());
        if (_server.has_value())
            ImGui::TextUnformatted("The server is running");
        else
            ImGui::TextUnformatted("The server is closed");
        ImGui::PopFont();
        // Values
        osc_manager().imgui_show_all_values();
    });
}

void ServerManager::set_port(int port)
{
    _port = port;
    if (_server.has_value() && _server->port() == _port) // Avoid recreating a server if this is already the right port
        return;
    if (_port < 0)
    {
        _server.reset();
        return;
    }

    try
    {
        _server = serv::Server{
            _port,
            {
                {
                    "/set",
                    [](serv::Request const& req) {
                        auto const name = req.get("name");
                        if (!name.has_value())
                        {
                            Log::error("Server", "When making a request to /set, you need to specify a name, e.g. http://localhost:1234/set?name=my_name&value=35");
                            return;
                        }
                        auto const value = req.get("value");
                        if (!value.has_value())
                        {
                            Log::error("Server", "When making a request to /set, you need to specify a value, e.g. http://localhost:1234/set?name=my_name&value=35");
                            return;
                        }
                        try
                        {
                            float const val = std::stof(*value);
                            osc_manager().set_value(OSCChannel{*name}, val);
                        }
                        catch (std::exception const&)
                        {
                            Log::error("Server", fmt::format("/set request had an invalid \"value\" param ({}). It must be a number.", *value));
                        }
                    },
                },
                {
                    "/",
                    [](serv::Request const& req) {
                        Log::error("Server", fmt::format("Invalid request ({}). Use /set instead", req.route()));
                    },
                },
            }
        };
    }
    catch (std::exception const& e)
    {
        Log::error("Server", fmt::format("Failed to open server: {}", e.what()));
        _server.reset();
    }
}

} // namespace Cool