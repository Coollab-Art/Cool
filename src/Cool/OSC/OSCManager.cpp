#include "OSCManager.h"
#include <exception>
#include <mutex>
#include <regex>
#include <sstream>
#include <thread>
#include <tl/expected.hpp>
#include "Cool/Exception/Exception.h"
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/Log/ErrorMessage.hpp"
#include "Cool/OSC/OSCChannel.h"
#include "Cool/OSC/OSCConnectionEndpoint.h"
#include "ip/UdpSocket.h"
#include "osc/OscPacketListener.h"

namespace Cool {

OSCManager::OSCManager()
    : _config_window{
          Cool::icon_fmt("OSC", ICOMOON_CONNECTION),
          Cool::ImGuiWindowConfig{
              .is_modal   = false,
              .start_open = false,
          }
      }
{}

OSCManager::~OSCManager()
{
    stop_listening();
}

// static auto list_channel_names(std::vector<std::pair<OSCChannel, float>> const& values) -> std::string
// {
//     std::stringstream ss;
//     for (auto const& [channel, _] : values)
//         ss << "- \"" << channel.name << "\"\n";
//     return ss.str();
// }

auto OSCManager::get_value(OSCChannel const& channel) -> float
{
    std::lock_guard lock{_s.values_mutex};
    for (auto const& pair : _s.values)
    {
        if (pair.first == channel)
            return pair.second;
    }

    _config_window.open();
    throw Cool::Exception{ErrorMessage{
        fmt::format(
            "We haven't received any OSC value named \"{}\"",
            channel.name
        )
    }};
}

void OSCManager::for_each_channel_that_has_changed(std::function<void(OSCChannel const&)> const& callback)
{
    std::lock_guard lock{_s.channels_that_have_changed_mutex};
    for (auto const& channel : _s.channels_that_have_changed)
        callback(channel);
    _s.channels_that_have_changed.clear();
}

void OSCManager::set_port(int port)
{
    set_connection_endpoint({port, _endpoint.ip_address});
}

void OSCManager::set_ip_address(std::string ip_address)
{
    set_connection_endpoint({_endpoint.port, std::move(ip_address)});
}

void OSCManager::set_connection_endpoint(OSCConnectionEndpoint endpoint)
{
    _endpoint = std::move(endpoint);
    // Recreate a listener using the new port and address
    stop_listening();
    // reset_values(); // Don't reset values, we want to have a continuity with the previous endpoint (and also, when starting the first endpoint, this would erase the values loaded by the serialization)
    start_listening();
}

auto OSCManager::get_connection_endpoint() const -> OSCConnectionEndpoint const&
{
    return _endpoint;
}

auto OSCManager::imgui_channel_widget(const char* label, OSCChannel& channel) -> bool
{
    bool b = false;
    // if (channel.name.empty() && !_s.values.empty())
    // {
    //     channel.name = _s.values[0].first; // Actually this is probably a bad idea
    //     b            = true;
    // }
    b |= ImGuiExtras::dropdown(label, &channel.name, [&](auto&& with_dropdown_entry) {
        std::lock_guard lock{_s.values_mutex};
        for (auto const& [channel, _] : _s.values)
            with_dropdown_entry(channel.name);
    });

    return b;
}

void OSCManager::imgui_error_message_for_invalid_endpoint(const char* extra_text) const
{
    if (_error_message_for_endpoint_creation.empty())
        return;

    ImGui::PushFont(Font::italic());
    ImGui::TextUnformatted(("OSC listener is OFF. " + _error_message_for_endpoint_creation + extra_text).c_str());
    ImGui::PopFont();
}

void OSCManager::imgui_show_all_values() // This function is also called by the ServerManager window to display the values
{
    imgui_values_table(); // TODO(OSC) For each channel, add a From Min / Max (and To Min / Max ?) to remap values + display a progress bar on the side to better see values changing, from 0 to 1
    ImGui::SameLine();
    imgui_button_to_reset_values();
}

void OSCManager::imgui_window()
{
    _config_window.show([&](bool /*is_opening*/) {
        imgui_select_connection_endpoint();
        imgui_show_all_values();
    });
}

void OSCManager::imgui_values_table()
{
    std::lock_guard       lock{_s.values_mutex};
    static constexpr auto table_flags = ImGuiTableFlags_RowBg
                                        | ImGuiTableFlags_Borders
                                        | ImGuiTableFlags_NoHostExtendX
                                        | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("OSC Values", 2, table_flags))
    {
        ImGui::PushFont(Cool::Font::monospace());
        for (auto const& [channel, value] : _s.values)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::BeginGroup();
            ImGui::TextUnformatted(channel.name.c_str());
            ImGui::Dummy({ImGui::GetContentRegionAvail().x, 0.f});
            ImGui::EndGroup();
            if (ImGui::BeginPopupContextItem(&channel.name))
            {
                if (ImGui::Button("Copy to clipboard"))
                {
                    ImGui::SetClipboardText(channel.name.c_str());
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::BeginGroup();
            ImGui::TextUnformatted(fmt::format("{}", value).c_str());
            ImGui::Dummy({ImGui::GetContentRegionAvail().x, 0.f});
            ImGui::EndGroup();
            if (ImGui::BeginPopupContextItem(&value))
            {
                if (ImGui::Button("Copy to clipboard"))
                {
                    ImGui::SetClipboardText(fmt::format("{}", value).c_str());
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::PopFont();
        ImGui::EndTable();
    }
}

void OSCManager::imgui_select_connection_endpoint()
{
    imgui_error_message_for_invalid_endpoint();
    // Port
    if (ImGuiExtras::input_port("Port", &_endpoint.port))
        set_port(_endpoint.port);
    // Address
    if (ImGuiExtras::input_text_with_dropdown(
            "IP Address", &_endpoint.ip_address, [&](auto&& with_dropdown_entry) {
                with_dropdown_entry(OSC_EVERY_AVAILABLE_ADDRESS);
                with_dropdown_entry("localhost");
            },
            ImGuiInputTextFlags_AutoSelectAll
        ))
    {
        set_ip_address(_endpoint.ip_address);
    }
}

void OSCManager::imgui_button_to_reset_values()
{
    if (values_are_empty())
        return;

    if (ImGuiExtras::button_with_text_icon(ICOMOON_UNDO))
        reset_values();
    ImGui::SetItemTooltip("Reset all values");
}

auto OSCManager::values_are_empty() const -> bool
{
    std::lock_guard lock{_s.values_mutex};
    return _s.values.empty();
}

void OSCManager::reset_values()
{
    std::lock_guard lock{_s.values_mutex};
    std::lock_guard lock2{_s.channels_that_have_changed_mutex};
    for (auto const& [name, _] : _s.values)
        _s.channels_that_have_changed.insert(OSCChannel{name});
    _s.values.clear();
}

namespace {
class OSCMessageHandler : public osc::OscPacketListener {
public:
    explicit OSCMessageHandler(internal::SharedWithThread& s)
        : _s{s}
    {}

private:
    void ProcessMessage(osc::ReceivedMessage const& m, IpEndpointName const&) override
    {
        for (auto arg = m.ArgumentsBegin(); arg != m.ArgumentsEnd(); ++arg)
        {
            if (arg->IsFloat())
                _s.set_value(OSCChannel{m.AddressPattern()}, arg->AsFloatUnchecked());
            if (arg->IsDouble())
                _s.set_value(OSCChannel{m.AddressPattern()}, static_cast<float>(arg->AsDoubleUnchecked()));
            if (arg->IsInt32())
                _s.set_value(OSCChannel{m.AddressPattern()}, static_cast<float>(arg->AsInt32Unchecked()));
            if (arg->IsInt64())
                _s.set_value(OSCChannel{m.AddressPattern()}, static_cast<float>(arg->AsInt64Unchecked()));
            if (arg->IsChar())
                _s.set_value(OSCChannel{m.AddressPattern()}, static_cast<float>(arg->AsCharUnchecked()));
            if (arg->IsBool())
                _s.set_value(OSCChannel{m.AddressPattern()}, arg->AsBoolUnchecked() ? 1.f : 0.f);
        }
    }

private:
    internal::SharedWithThread& _s; // NOLINT(*avoid-const-or-ref-data-members)
};
} // namespace

void internal::SharedWithThread::set_value(OSCChannel const& channel, float value)
{
    {
        std::lock_guard lock{values_mutex};
        size_t const    index{find_index(channel)};
        values[index].second = value;
    }
    {
        std::lock_guard lock{channels_that_have_changed_mutex};
        channels_that_have_changed.insert(channel);
    }
}

auto internal::SharedWithThread::find_index(OSCChannel const& channel) -> size_t
{
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i].first == channel)
            return i;
    }
    values.emplace_back(channel, 0.f);
    std::sort(values.begin(), values.end(), [](auto const& a, auto const& b) {
        return a.first < b.first;
    });
    return values.size() - 1;
}

void OSCManager::set_value(OSCChannel const& channel, float value)
{
    _s.set_value(channel, value);
}

static auto get_ip_endpoint_name(OSCConnectionEndpoint const& endpoint) -> tl::expected<IpEndpointName, std::string>
{
    // Check port
    if (endpoint.port < 0)
        return tl::make_unexpected("Select a valid port to start listening to OSC messages.");

    // Check address
    if (endpoint.ip_address == OSC_EVERY_AVAILABLE_ADDRESS)
        return IpEndpointName{IpEndpointName::ANY_ADDRESS, endpoint.port};
    if (endpoint.ip_address == "localhost")
        return IpEndpointName{"localhost", endpoint.port};

    static const auto ipv4_regexp   = std::regex{R"((\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3}))"};
    auto              match_results = std::smatch{};
    if (std::regex_match(endpoint.ip_address, match_results, ipv4_regexp))
        return IpEndpointName{std::stoi(match_results[1]), std::stoi(match_results[2]), std::stoi(match_results[3]), std::stoi(match_results[4]), endpoint.port};

    return tl::make_unexpected(fmt::format(R"STR(Select a valid IPv4 address to start listening to OSC messages.
Allowed addresses are "{}", "localhost", or something of the form "192.168.1.1".)STR",
                                           OSC_EVERY_AVAILABLE_ADDRESS));
}

void OSCManager::start_listening()
{
    auto const endpoint_name = get_ip_endpoint_name(_endpoint);
    if (!endpoint_name.has_value())
    {
        _error_message_for_endpoint_creation = endpoint_name.error();
        return;
    }
    _error_message_for_endpoint_creation = "";

    std::atomic<bool> thread_has_finished_init{false};
    _thread = std::thread([&]() {
        try
        {
            auto listener = OSCMessageHandler{_s};
            auto socket   = UdpListeningReceiveSocket{*endpoint_name, &listener};
            _stop_thread  = [&]() {
                socket.AsynchronousBreak();
            };
            thread_has_finished_init.store(true);
            socket.Run();
        }
        catch (std::exception const& e)
        {
            _stop_thread = []() {
            };
            _error_message_for_endpoint_creation = fmt::format("Select a valid IPv4 address to start listening to OSC messages. ({})", e.what());
            thread_has_finished_init.store(true);
        }
    });
    while (!thread_has_finished_init.load()) // Block until the thread is ready, otherwise we might continue and call stop_listening() when the thread is not properly setup, which will crash.
    {
    }
}

void OSCManager::stop_listening()
{
    if (!_thread.has_value())
        return;
    _stop_thread();
    _stop_thread = []() {};
    if (_thread->joinable())
        _thread->join();
    _thread.reset();
}

} // namespace Cool