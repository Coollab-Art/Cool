#pragma once
#include <Cool/ImGui/IcoMoonCodepoints.h>
#include <Audio/Audio.hpp>
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/Time/Clock.h"
#include "Cool/Utils/Cached.h"
#include "Cool/WebGPU/Buffer.h"
#include "internal/AudioInput_Device.h"
#include "internal/AudioInput_File.h"
#include "internal/IAudioInput.h"

namespace ser20 {

template<class Archive>
void serialize(Archive& archive, Audio::PlayerProperties& properties)
{
    archive(
        ser20::make_nvp("Volume", properties.volume),
        ser20::make_nvp("Is muted", properties.is_muted),
        ser20::make_nvp("Does loop", properties.does_loop)
    );
}

} // namespace ser20

namespace Cool {

enum class AudioInputMode {
    Device,
    File,
};

class AudioManager {
public:
    AudioManager();

    /// Returns an array containing the raw audio samples, with values between -1 and 1. There is only one channel, so each sample corresponds to the next moment in time.
    [[nodiscard]] auto waveform() const -> std::vector<float> const&;
    /// Returns a buffer of floats. Contains values between -1 and 1.
    [[nodiscard]] auto waveform_buffer() const -> Buffer const&;
    /// Returns an array where the values correspond to the amplitudes of frequencies evenly spaced between 0 and sample_rate / 2. Where sample_rate is the sample rate of the analysed audio.
    [[nodiscard]] auto spectrum() const -> Audio::Spectrum const&;
    /// Returns a buffer of floats. Contains values between 0 and +infinity.
    [[nodiscard]] auto spectrum_buffer() const -> Buffer const&;
    ///
    [[nodiscard]] auto volume() const -> float;
    ///
    [[nodiscard]] auto wants_to_display_audio_spectrum_as_bars() const -> bool { return _spectrum_display_as_bars; }

    void sync_with_clock(Cool::Clock const&, bool force_sync_time = false);
    /// Must be called every frame. Will do some internal bookkeeping, and call `on_audio_data_changed` iff the audio data has changed (e.g. music player is playing, or a microphone is beeing used).
    void update(std::function<void()> const& on_audio_data_changed);

    void set_current_input_mode(AudioInputMode);

    void open_imgui_window() { _window.open(); }
    void imgui_window();

    void set_force_mute(bool mute) { _file_input.set_force_mute(mute); }

private:
    [[nodiscard]] auto nb_frames_for_feature_computation(float window_size_in_seconds) const -> int64_t;

    void invalidate_caches();

    /// Returns the current audio input in use, as per `_current_input_mode`
    [[nodiscard]] auto current_input() -> internal::IAudioInput&;
    [[nodiscard]] auto current_input() const -> internal::IAudioInput const&;

private:
    internal::AudioInput_File   _file_input{};
    internal::AudioInput_Device _device_input{};
    AudioInputMode              _current_input_mode{AudioInputMode::Device};
    bool                        _audio_data_has_been_invalidated{false}; // Used to signal that `on_audio_data_changed` needs to be called during `update()`.

    mutable Cached<std::vector<float>>    _current_waveform{};
    mutable Cached<Audio::Spectrum>       _current_spectrum{};
    mutable Cached<float>                 _current_volume{};
    mutable Cached<std::optional<Buffer>> _current_spectrum_buffer{};
    mutable Cached<std::optional<Buffer>> _current_waveform_buffer{};

    float _window_size_in_seconds_for_waveform{0.05f};
    float _window_size_in_seconds_for_spectrum{0.015f};
    float _window_size_in_seconds_for_volume{0.2f};

    float _spectrum_max_frequency_in_hz{2500.f};
    float _spectrum_height_scale{1.f};
    bool  _spectrum_display_as_bars{true};

    ImGuiWindow _window{icon_fmt("Audio", ICOMOON_MUSIC)};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            ser20::make_nvp("Window", _window),
            ser20::make_nvp("File input", _file_input),
            ser20::make_nvp("Device input", _device_input),
            ser20::make_nvp("Current input mode", _current_input_mode),
            ser20::make_nvp("Window size for waveform", _window_size_in_seconds_for_waveform),
            ser20::make_nvp("Window size for spectrum", _window_size_in_seconds_for_spectrum),
            ser20::make_nvp("Window size for volume", _window_size_in_seconds_for_volume),
            ser20::make_nvp("Spectrum max frequency in Hz", _spectrum_max_frequency_in_hz),
            ser20::make_nvp("Spectrum height scale", _spectrum_height_scale),
            ser20::make_nvp("Spectrum display as bars", _spectrum_display_as_bars)
        );
    }
    template<class Archive>
    void load(Archive& archive)
    {
        archive(
            _window,
            _file_input,
            _device_input,
            _current_input_mode,
            _window_size_in_seconds_for_waveform,
            _window_size_in_seconds_for_spectrum,
            _window_size_in_seconds_for_volume,
            _spectrum_max_frequency_in_hz,
            _spectrum_height_scale,
            _spectrum_display_as_bars
        );
        _device_input.stop(); // Device input always automatically starts when it is created, so we need to stop it.
        _file_input.stop();
        current_input().start();
    }
};

} // namespace Cool