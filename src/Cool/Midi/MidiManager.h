#pragma once
#include <chrono>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/Time/Time.hpp"
#include "MidiChannel.h"
#include "RtMidiW/RtMidiW.hpp"
#include "ser20/types/array.hpp"
#include "ser20/types/chrono.hpp"

namespace Cool {

class MidiValues {
public:
    auto get_map(MidiChannelKind kind) -> std::unordered_map<int, float>&;
    auto get_map(MidiChannelKind kind) const -> std::unordered_map<int, float> const&;

    auto last_button_pressed() const -> int;
    auto last_last_button_pressed() const -> int;
    auto time_since_last_button_pressed() const -> Time;
    void set_last_button_pressed(int button);

private:
    std::array<std::unordered_map<int, float>, static_cast<size_t>(MidiChannelKind::COUNT)> _maps{};

    int                                   _last_button_pressed{-1};
    int                                   _last_last_button_pressed{-1};
    std::chrono::steady_clock::time_point _time_of_last_button_pressed{std::chrono::steady_clock::now()};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Values", _maps),
            ser20::make_nvp("Last button pressed", _last_button_pressed),
            ser20::make_nvp("Last last button pressed", _last_last_button_pressed),
            ser20::make_nvp("Time of last button pressed", _time_of_last_button_pressed)
        );
    }
};

class MidiManager {
public:
    /// Returns a number between 0 and 1 representing the current state of the controller.
    /// /!\ Note that if a knob hasn't been used yet we won't know it's current state and will return 0 instead.
    [[nodiscard]] auto get_value(MidiChannel const&) const -> float;
    /// The `callback` will be called once for each channel whose value has changed since the last call to `for_each_channel_that_has_changed()`.
    void               for_each_channel_that_has_changed(std::function<void(MidiChannel const&)> const& callback);
    [[nodiscard]] auto last_button_pressed_has_changed() -> bool;

    /// You can use this if you want to emulate a midi keyboard.
    /// `value` should be between 0 and 1, since this is what people expect to receive back from `get_value()`.
    void set_value(MidiChannel const&, float value);
    void set_all_but_one_to(float value, int ignored_index, MidiChannelKind);
    void reset_all(MidiChannelKind);

    void open_config_window() { _config_window.open(); }

    void imgui_window();
    void imgui_visualize_channels();
    void imgui_reset_all(MidiChannelKind, std::string_view label);
    void imgui_controllers_dropdown();
    void imgui_emulate_midi_keyboard();

    auto all_values() const -> auto const& { return _all_values; }
    auto all_values() -> auto& { return _all_values; }

private:
    friend auto midi_manager() -> MidiManager&;
    MidiManager(); // Use `mini_manager()` to get the unique instance of MidiManager.

    friend class AppManager;
    void check_for_devices();

    void open_port(unsigned int index);
    void close_port();

    auto               get_value_no_locking(MidiChannel const& channel) const -> float;
    [[nodiscard]] auto max_index(MidiChannelKind) const -> int;

    static void midi_callback(double delta_time, std::vector<unsigned char>* message, void* user_data);
    static void midi_error_callback(RtMidiError::Type type, std::string const& error_text, void* user_data);

private:
    mutable std::mutex          _mutex{};
    std::optional<RtMidiIn>     _midi{}; // Optional because the creation might fail and throw an exception: http://www.music.mcgill.ca/~gary/rtmidi/index.html#start
    std::string                 _port_name{};
    std::optional<unsigned int> _port_index{};
    MidiValues                  _all_values{};
    Cool::ImGuiWindow           _config_window;
    std::set<MidiChannel>       _channels_that_have_changed{};
    bool                        _last_button_pressed_has_changed{false};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Window", _config_window),
            ser20::make_nvp("All Values", _all_values)
        );
    }
};

inline auto midi_manager() -> MidiManager&
{
    static auto instance = MidiManager{};
    return instance;
}

} // namespace Cool