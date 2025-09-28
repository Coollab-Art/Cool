#pragma once
#include <mutex>
#include <set>
#include "Cool/ImGui/ImGuiWindow.h"
#include "OSCChannel.h"
#include "OSCConnectionEndpoint.h"

namespace Cool {

namespace internal {
struct SharedWithThread {
    mutable std::mutex                        values_mutex{};
    std::vector<std::pair<OSCChannel, float>> values{};
    std::mutex                                channels_that_have_changed_mutex{};
    std::set<OSCChannel>                      channels_that_have_changed{};

    void set_value(OSCChannel const& channel, float value);

private:
    auto find_index(OSCChannel const& channel) -> size_t;
};
} // namespace internal

class OSCManager {
public:
    OSCManager(OSCManager const&)                = delete;
    OSCManager& operator=(OSCManager const&)     = delete;
    OSCManager(OSCManager&&) noexcept            = delete;
    OSCManager& operator=(OSCManager&&) noexcept = delete;
    ~OSCManager();

    /// Throws a Cool::Exception if the channel doesn't correspond to any received value.
    auto get_value(OSCChannel const&) -> float;
    /// The `callback` will be called once for each channel whose value has changed since the last call to `for_each_channel_that_has_changed()`.
    void for_each_channel_that_has_changed(std::function<void(OSCChannel const&)> const& callback);

    void set_port(int port);
    void set_ip_address(std::string ip_address);
    void set_connection_endpoint(OSCConnectionEndpoint);
    auto get_connection_endpoint() const -> OSCConnectionEndpoint const&;

    auto imgui_channel_widget(const char* label, OSCChannel&) -> bool;
    void imgui_error_message_for_invalid_endpoint(const char* extra_text = "") const;
    void imgui_show_all_values();

    void open_config_window() { _config_window.open(); }
    void imgui_window();

    void set_value(OSCChannel const&, float);

private:
    OSCManager(); // This is private, because all instances of OSCManager must be guaranteed to have a fixed address in memory. Use osc_manager() to access the global OSCManager.
    friend auto osc_manager() -> OSCManager&;

    void start_listening();
    void stop_listening();

    void imgui_values_table();
    void imgui_select_connection_endpoint();
    void imgui_button_to_reset_values();

    auto values_are_empty() const -> bool;
    void reset_values();

private:
    internal::SharedWithThread _s{};
    Cool::ImGuiWindow          _config_window;
    std::optional<std::thread> _thread{};
    std::function<void()>      _stop_thread{[]() {}};
    OSCConnectionEndpoint      _endpoint{};
    std::string                _error_message_for_endpoint_creation{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void save(Archive& archive) const
    {
        std::lock_guard lock{_s.values_mutex};
        archive(
            ser20::make_nvp("Window", _config_window),
            ser20::make_nvp("Endpoint", _endpoint),
            ser20::make_nvp("Values", _s.values)
        );
    }
    template<class Archive>
    void load(Archive& archive)
    {
        std::lock_guard lock{_s.values_mutex};
        auto            endpoint = OSCConnectionEndpoint{};
        archive(
            ser20::make_nvp("Window", _config_window),
            ser20::make_nvp("Endpoint", endpoint),
            ser20::make_nvp("Values", _s.values)
        );
        set_connection_endpoint(endpoint); // This will (re)start the server
    }
};

inline auto osc_manager() -> OSCManager&
{
    static auto instance = OSCManager{};
    return instance;
}

} // namespace Cool