#include "MidiManager.h"
#include <chrono>
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/ImGui/markdown.h"
#include "Cool/Time/time_formatted_hms.h"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "MidiChannel.h"
#include "imgui.h"

namespace Cool {

auto MidiValues::get_map(MidiChannelKind kind) -> std::unordered_map<int, float>&
{
    auto const index = static_cast<size_t>(kind);
    if (index >= static_cast<size_t>(MidiChannelKind::COUNT))
    {
        assert(false);
        return _maps[0];
    }
    return _maps[index];
}
auto MidiValues::get_map(MidiChannelKind kind) const -> std::unordered_map<int, float> const&
{
    auto const index = static_cast<size_t>(kind);
    if (index >= static_cast<size_t>(MidiChannelKind::COUNT))
    {
        assert(false);
        return _maps[0];
    }
    return _maps[index];
}

auto MidiValues::last_button_pressed() const -> int
{
    return _last_button_pressed;
}
auto MidiValues::last_last_button_pressed() const -> int
{
    return _last_last_button_pressed;
}
auto MidiValues::time_since_last_button_pressed() const -> Time
{
    return std::chrono::steady_clock::now() - _time_of_last_button_pressed;
}
void MidiValues::set_last_button_pressed(int button)
{
    _last_last_button_pressed    = _last_button_pressed;
    _last_button_pressed         = button;
    _time_of_last_button_pressed = std::chrono::steady_clock::now();
}

MidiManager::MidiManager()
    : _config_window{
          Cool::icon_fmt("MIDI", ICOMOON_EQUALIZER2),
          Cool::ImGuiWindowConfig{
              .is_modal   = false,
              .start_open = false,
          }
      }
{
    try
    {
        _midi.emplace();
        _midi->setErrorCallback(&midi_error_callback);
        _midi->setCallback(&midi_callback, this);
        _midi->ignoreTypes(true, true, true); // ignore sysex, timing, and active sensing messages.
    }
    catch (std::exception const& e)
    {
        Log::error("MIDI Error", e.what());
    }
}

auto MidiManager::get_value(MidiChannel const& channel) const -> float
{
    std::lock_guard lock{_mutex};
    return get_value_no_locking(channel);
}

auto MidiManager::get_value_no_locking(MidiChannel const& channel) const -> float
{
    auto const& map = _all_values.get_map(channel.kind);
    auto const  it  = map.find(channel.index);
    if (it == map.end())
        return 0.f;
    return it->second;
}

void MidiManager::set_value(MidiChannel const& channel, float value)
{
    std::lock_guard lock{_mutex};
    auto&           current_value = _all_values.get_map(channel.kind)[channel.index];
    if (current_value == value)
        return;
    current_value = value;
    _channels_that_have_changed.insert(channel);
}

void MidiManager::set_all_but_one_to(float value, int ignored_index, MidiChannelKind kind)
{
    std::lock_guard lock{_mutex};

    auto& map = _all_values.get_map(kind);
    for (auto& [index, current_value] : map)
    {
        if (current_value == value || index == ignored_index)
            continue;
        current_value = value;
        _channels_that_have_changed.insert(MidiChannel{.index = index, .kind = kind});
    }
}

void MidiManager::reset_all(MidiChannelKind kind)
{
    std::lock_guard lock{_mutex};

    auto& map = _all_values.get_map(kind);
    for (auto const& [index, current_value] : map)
    {
        if (current_value != 0.f) // Optimisation: values that were already 0 don't need to signal that they have changed
            _channels_that_have_changed.insert(MidiChannel{.index = index, .kind = kind});
    }
    map.clear();
}

void MidiManager::for_each_channel_that_has_changed(std::function<void(MidiChannel const&)> const& callback)
{
    std::lock_guard lock{_mutex};
    for (auto const& channel : _channels_that_have_changed)
        callback(channel);
    _channels_that_have_changed.clear();
}

auto MidiManager::last_button_pressed_has_changed() -> bool
{
    std::lock_guard lock{_mutex};
    bool const      res              = _last_button_pressed_has_changed;
    _last_button_pressed_has_changed = false;
    return res;
}

void MidiManager::check_for_devices()
{
    if (!_midi.has_value())
        return;

    try
    {
        auto const port_count = _midi->getPortCount();

        if (_port_index.has_value()
            && *_port_index < port_count
            && _midi->getPortName(*_port_index) == _port_name)
        {
            return; // The port we have opened still exists and hasn't been moved because of the creation of new ports.
        }

        if (port_count > 0)
            open_port(0);
        else
            close_port();
    }
    catch (std::exception const& e)
    {
        Log::error("MIDI Error", e.what());
    }
}

void MidiManager::midi_callback(double /* delta_time */, std::vector<unsigned char>* message, void* user_data)
{
    if (message->size() < 3)
        return;
    auto& This = *static_cast<MidiManager*>(user_data);

    auto const channel_kind  = (*message)[0];
    auto const channel_idx   = (*message)[1];
    auto const channel_value = (*message)[2];

    if (channel_kind == 176) // Slider / Knob / Fader
    {
        This.set_value(
            MidiChannel{.index = channel_idx, .kind = MidiChannelKind::Slider},
            static_cast<float>(channel_value) / 127.f
        );
    }
    else if (channel_kind == 144) // Button pressed
    {
        if (This._all_values.last_button_pressed() != channel_idx)
        {
            This._all_values.set_last_button_pressed(channel_idx);
            This._last_button_pressed_has_changed = true;
        }
        This.set_value(
            MidiChannel{.index = channel_idx, .kind = MidiChannelKind::ButtonWhilePressed},
            1.f
        );
        This.set_value(
            MidiChannel{.index = channel_idx, .kind = MidiChannelKind::ButtonToggle},
            1.f - This.get_value(MidiChannel{.index = channel_idx, .kind = MidiChannelKind::ButtonToggle})
        );
        This.set_all_but_one_to(0.f, channel_idx, MidiChannelKind::ButtonSelector);
        This.set_value(
            MidiChannel{.index = channel_idx, .kind = MidiChannelKind::ButtonSelector},
            1.f
        );
    }
    else if (channel_kind == 128) // Button released
    {
        This.set_value(
            MidiChannel{.index = channel_idx, .kind = MidiChannelKind::ButtonWhilePressed},
            0.f
        );
    }
    else
    {
        assert(false);
        This.set_value(
            MidiChannel{.index = channel_idx, .kind = MidiChannelKind::Slider},
            static_cast<float>(channel_value) / 127.f
        );
    }
}

void MidiManager::midi_error_callback(RtMidiError::Type type, std::string const& error_text, void* /* user_data */)
{
    if (type == RtMidiError::Type::DRIVER_ERROR)
    {
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Warning,
            .title   = "MIDI",
            .content = fmt::format("Failed to connect to the device. Maybe it is already used in another software? You will need to restart {} to try to reconnect to the MIDI device.\n\n{}", COOL_APP_NAME, error_text),
        });
    }
    else
    {
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Warning,
            .title   = "MIDI",
            .content = error_text,
        });
    }
}

void MidiManager::open_port(unsigned int index)
{
    if (!_midi.has_value())
        return;

    try
    {
        _midi->closePort();
        _midi->openPort(index);
        _port_name  = _midi->getPortName(index);
        _port_index = index;
    }
    catch (std::exception const& e)
    {
        Log::error("MIDI Error", e.what());
    }
}

void MidiManager::close_port()
{
    if (!_midi.has_value())
        return;

    try
    {
        _midi->closePort();
        _port_name.clear();
        _port_index.reset();
    }
    catch (std::exception const& e)
    {
        Log::error("MIDI Error", e.what());
    }
}

auto MidiManager::max_index(MidiChannelKind kind) const -> int
{
    auto const& map = _all_values.get_map(kind);
    auto const  it  = std::max_element(map.begin(), map.end(), [](auto const& p1, auto const& p2) {
        return p1.first < p2.first; // Compare the indices
    });

    if (it == map.end())
        return 0;
    return it->first;
}

void MidiManager::imgui_window()
{
    _config_window.show([&](bool /*is_opening*/) {
        imgui_controllers_dropdown();
        ImGui::NewLine();
        imgui_visualize_channels();
    });
}

void MidiManager::imgui_visualize_channels()
{
    ImGui::PushFont(Font::italic());
    ImGuiExtras::markdown("*Use your knob / fader / button and you will see the value changing in the histograms below. You can then hover it to see the index that you need to specify to select a MIDI channel. NOTE: if you want to use a Button, don't forget to change the Channel Kind on the MIDI input!*");
    ImGui::PopFont();

    for (int k = 0; k < static_cast<int>(MidiChannelKind::COUNT); ++k)
    {
        auto const kind = static_cast<MidiChannelKind>(k);

        auto values = std::vector<float>{};
        {
            std::lock_guard lock{_mutex};
            values.resize(static_cast<size_t>(max_index(kind) + 1));
            for (size_t i = 0; i < values.size(); ++i)
                values[i] = get_value_no_locking({.index = static_cast<int>(i), .kind = kind});
        }
        const char* label = user_facing_string(kind);
        ImGui::PushID(label);
        ImGui::PlotHistogram("", values.data(), static_cast<int>(values.size()), 0, label, 0.f, 1.f, ImVec2(0, 80.0f));
        ImGui::SameLine();
        imgui_reset_all(kind, label);
        ImGui::PopID();
    }
    ImGui::TextUnformatted(fmt::format("Last button pressed: {}", all_values().last_button_pressed()).c_str());
    ImGui::TextUnformatted(fmt::format("Last last button pressed: {}", all_values().last_last_button_pressed()).c_str());
    ImGui::TextUnformatted(fmt::format("Time since last button pressed: {}", time_formatted_hms(all_values().time_since_last_button_pressed())).c_str());
}

void MidiManager::imgui_reset_all(MidiChannelKind kind, std::string_view label)
{
    if (ImGuiExtras::button_with_text_icon(ICOMOON_UNDO))
        reset_all(kind);
    ImGui::SetItemTooltip("%s", fmt::format("Reset all {}", label).c_str());
}

void MidiManager::imgui_controllers_dropdown()
{
    if (!_midi.has_value())
        return;

    try
    {
        if (ImGui::BeginCombo("Controller", _port_name.c_str()))
        {
            for (unsigned int i = 0; i < _midi->getPortCount(); ++i)
            {
                if (ImGui::Selectable(_midi->getPortName(i).c_str()))
                    open_port(i);
            }
            ImGui::EndCombo();
        }
    }
    catch (std::exception const& e)
    {
        Log::error("MIDI Error", e.what());
    }
}

void MidiManager::imgui_emulate_midi_keyboard()
{
    static auto values        = std::vector<float>(31);
    static auto buttons_count = 10;
    for (int i = 0; i < static_cast<int>(values.size()); ++i)
    {
        ImGui::PushID(i);
        if (ImGui::VSliderFloat("", ImVec2(30, 160), &values[static_cast<size_t>(i)], 0.f, 1.f, ""))
        {
            std::vector<unsigned char> message{176 /*indicating that this is a slider / knob*/, static_cast<unsigned char>(i), static_cast<unsigned char>(values[static_cast<size_t>(i)] * 127.f)};
            midi_callback(0.f, &message, this);
        }
        ImGui::SetItemTooltip("%i", i);
        if (i != static_cast<int>(values.size()) - 1)
            ImGui::SameLine();
        ImGui::PopID();
    }
    ImGui::TextUnformatted("Buttons:");
    for (int i = 0; i < buttons_count; ++i)
    {
        ImGui::SameLine();
        ImGui::Button(fmt::format("{}", i).c_str());
        if (ImGui::IsItemActivated())
        {
            auto message = std::vector<unsigned char>{144 /*button pressed*/, static_cast<unsigned char>(i), 0};
            midi_callback(0., &message, this);
        }
        if (ImGui::IsItemDeactivated())
        {
            auto message = std::vector<unsigned char>{128 /*button released*/, static_cast<unsigned char>(i), 0};
            midi_callback(0., &message, this);
        }
    }
    if (ImGui::Button("Add fader"))
        values.push_back(0.f);
    ImGui::SameLine();
    if (ImGui::Button("Remove fader") && !values.empty())
        values.pop_back();
    if (ImGui::Button("Add button"))
        buttons_count++;
    ImGui::SameLine();
    if (ImGui::Button("Remove button") && buttons_count > 0)
        buttons_count--;
}

} // namespace Cool