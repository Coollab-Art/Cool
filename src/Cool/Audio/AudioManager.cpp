#include "AudioManager.h"
// #include <glpp-extended/src/TextureLayout.h>
#include <imgui.h>
#include <optional>
#include "Cool/Audio/AudioManager.h"
#include "Cool/DebugOptions/DebugOptions.h"

namespace Cool {

// TODO(Audio) Average the spectrum across several frames, to make it look smoother.

AudioManager::AudioManager()
{
    current_input().start();
}

auto AudioManager::current_input() -> internal::IAudioInput&
{
    if (_current_input_mode == AudioInputMode::Device)
        return _device_input;
    return _file_input;
}
auto AudioManager::current_input() const -> internal::IAudioInput const&
{
    if (_current_input_mode == AudioInputMode::Device)
        return _device_input;
    return _file_input;
}

auto AudioManager::volume() const -> float
{
    return _current_volume.get_value([&]() {
        if (Cool::DebugOptions::log_when_computing_audio_features())
            Log::info("Audio", "Computing volume");

        auto frames = std::vector<float>{};
        current_input().for_each_audio_frame(nb_frames_for_feature_computation(_window_size_in_seconds_for_volume), [&](float frame) {
            frames.push_back(frame);
        });
        return Audio::compute_volume(frames);
    });
}

auto AudioManager::waveform() const -> std::vector<float> const&
{
    return _current_waveform.get_value([&]() {
        if (Cool::DebugOptions::log_when_computing_audio_features())
            Log::info("Audio", "Computing waveform");

        auto waveform = std::vector<float>{};
        current_input().for_each_audio_frame(nb_frames_for_feature_computation(_window_size_in_seconds_for_waveform), [&](float sample) {
            waveform.push_back(sample);
        });
        return waveform;
    });
}

auto AudioManager::spectrum() const -> Audio::Spectrum const&
{
    return _current_spectrum.get_value([&]() {
        if (Cool::DebugOptions::log_when_computing_audio_features())
            Log::info("Audio", "Computing spectrum");

        auto const N = nb_frames_for_feature_computation(_window_size_in_seconds_for_spectrum);
        return Audio::fourier_transform(
            static_cast<size_t>(N),
            [&](std::function<void(float)> const& callback) {
                int i = 0;
                current_input().for_each_audio_frame(N, [&](float frame) {
                    float const t = static_cast<float>(i) / static_cast<float>(N - 1);
                    i++;
                    float const window = 1.f - std::abs(2.f * t - 1.f); // Applying a window allows us to reduce "spectral leakage" https://digitalsoundandmusic.com/2-3-11-windowing-functions-to-eliminate-spectral-leakage/  // TODO(Audio) Better windowing function? Or give the option to choose which windowing function to use? (We're gonna create the widget anyways to test. If we do give the option, we need to specify next to each window what it is good at.)  We can pick from https://digitalsoundandmusic.com/2-3-11-windowing-functions-to-eliminate-spectral-leakage/
                    callback(frame * window * _spectrum_height_scale);
                });
            },
            static_cast<float>(current_input().sample_rate()),
            _spectrum_max_frequency_in_hz
        );
    });
}

auto AudioManager::waveform_buffer() const -> Buffer const&
{
    return *_current_waveform_buffer.get_value([&]() {
        return buffer_from_data(waveform().empty() ? std::vector{0.f} : waveform());
    });
}

auto AudioManager::spectrum_buffer() const -> Buffer const&
{
    return *_current_spectrum_buffer.get_value([&]() {
        return buffer_from_data(spectrum().data.empty() ? std::vector{0.f} : spectrum().data);
    });
}

auto AudioManager::nb_frames_for_feature_computation(float window_size_in_seconds) const -> int64_t
{
    return static_cast<int64_t>(
        current_input().sample_rate() * window_size_in_seconds
    );
}

void AudioManager::sync_with_clock(Cool::Clock const& clock, bool force_sync_time)
{
    if (_current_input_mode != AudioInputMode::File)
        return;

    if (!clock.is_playing()
        || clock.is_being_forced_to_not_respect_realtime() // Time is paused or frozen because the user is using the input text of the timeline to set the time value, or dragging the timeline.
    )
    {
        Audio::player().pause();
    }
    else
    {
        Audio::player().play();
    }

    force_sync_time |= !Audio::player().is_playing(); // While paused, we can force syncing without hearing audio glitches. And this helps making it smoother when we drag time on the timeline: the audio updates immediately and not through increments of 0.5 seconds.
    if (force_sync_time
        || std::abs(clock.time().as_seconds_double() - Audio::player().get_time()) > 0.5 // Syncing every frame sounds really bad, so we only sync when a gap has appeared.
    )
    {
        // NB: We sync the player even when it is paused because the audio features need the player to always be synced with the clock in order to get the data from the current time.
        if (Audio::player().set_time(clock.time().as_seconds_double()))
            _audio_data_has_been_invalidated = true;
    }
}

void AudioManager::invalidate_caches()
{
    _current_waveform.invalidate_cache();
    _current_spectrum.invalidate_cache();
    _current_waveform_buffer.invalidate_cache();
    _current_spectrum_buffer.invalidate_cache();
    _current_volume.invalidate_cache();
}

void AudioManager::update(std::function<void()> const& on_audio_data_changed)
{
    _audio_data_has_been_invalidated |= current_input().is_playing();
    if (_audio_data_has_been_invalidated)
    {
        invalidate_caches();
        on_audio_data_changed();
        _audio_data_has_been_invalidated = false;
    }
    current_input().update();
    if (_current_input_mode == AudioInputMode::Device)
    {
        _device_input.set_nb_of_retained_samples(
            static_cast<size_t>(std::max(
                std::max(
                    nb_frames_for_feature_computation(_window_size_in_seconds_for_waveform),
                    nb_frames_for_feature_computation(_window_size_in_seconds_for_spectrum)
                ),
                nb_frames_for_feature_computation(_window_size_in_seconds_for_volume)
            ))
        );
    }
}

static auto to_string(AudioInputMode mode) -> const char*
{
    switch (mode)
    {
    case AudioInputMode::File:
        return ICOMOON_FILE_MUSIC " File";
    case AudioInputMode::Device:
        return ICOMOON_MIC " Device";
    }
    return "";
}

void AudioManager::set_current_input_mode(AudioInputMode mode)
{
    if (mode == _current_input_mode)
        return;
    current_input().stop();
    _current_input_mode = mode;
    current_input().start();
    _audio_data_has_been_invalidated = true;
}

static auto imgui_spectrum_display_as_bars(bool* val) -> bool
{
    bool b = false;
    if (ImGui::BeginCombo("Display as", *val ? "Bars" : "Lines"))
    {
        {
            bool const is_selected = *val;
            if (ImGui::Selectable("Bars", is_selected))
            {
                *val = true;
                b    = true;
            }
            if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                ImGui::SetItemDefaultFocus();
        }
        {
            bool const is_selected = !*val;
            if (ImGui::Selectable("Lines", is_selected))
            {
                *val = false;
                b    = true;
            }
            if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return b;
}

void AudioManager::imgui_window()
{
    bool const needs_to_highlight_error = current_input().does_need_to_highlight_error();
    if (needs_to_highlight_error)
        _window.open();

    _window.show([&](bool /*is_opening*/) {
        ImGui::SeparatorText("Input Selection");
        // Select the input mode
        if (ImGui::BeginCombo("Input Mode", to_string(_current_input_mode)))
        {
            int const mode = static_cast<int>(_current_input_mode);
            for (int i = 0; i < 2; ++i)
            {
                bool const is_selected = i == mode;
                if (ImGui::Selectable(to_string(static_cast<AudioInputMode>(i)), is_selected))
                    set_current_input_mode(static_cast<AudioInputMode>(i));

                if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        _audio_data_has_been_invalidated |= current_input().imgui(needs_to_highlight_error);

        ImGui::NewLine();
        if (ImGui::BeginTabBar("##audio_features"))
        {
            if (ImGui::BeginTabItem("Volume"))
            {
                ImGuiExtras::progress_bar(volume(), {0.f, 0.f});
                _audio_data_has_been_invalidated |= ImGui::SliderFloat("Window size##Volume", &_window_size_in_seconds_for_volume, 0.f, 1.f, "%.3f seconds");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Waveform"))
            {
                ImGui::PlotLines(
                    "##Waveform",
                    waveform().data(),
                    static_cast<int>(waveform().size()),
                    0, nullptr,
                    -1.f, 1.f, // Values are between -1 and 1
                    {0.f, 100.f}
                );
                _audio_data_has_been_invalidated |= ImGui::SliderFloat("Window size##Waveform", &_window_size_in_seconds_for_waveform, 0.f, 0.74f, "%.3f seconds");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Spectrum"))
            {
                if (_spectrum_display_as_bars)
                {
                    ImGui::PlotHistogram(
                        "##Spectrum",
                        spectrum().data.data(),
                        static_cast<int>(spectrum().data.size()),
                        0, nullptr,
                        0.f, 1.f,
                        {0.f, 100.f}
                    );
                }
                else
                {
                    ImGui::PlotLines(
                        "##Spectrum",
                        spectrum().data.data(),
                        static_cast<int>(spectrum().data.size()),
                        0, nullptr,
                        0.f, 1.f,
                        {0.f, 100.f}
                    );
                }
                _audio_data_has_been_invalidated |= imgui_spectrum_display_as_bars(&_spectrum_display_as_bars);
                _audio_data_has_been_invalidated |= ImGui::SliderFloat("Window size##Spectrum", &_window_size_in_seconds_for_spectrum, 0.f, 0.5f, "%.3f seconds");
                _audio_data_has_been_invalidated |= ImGui::SliderFloat("Max pitch displayed", &_spectrum_max_frequency_in_hz, 0.f, 22000.f, "%.0f Hertz");
                _audio_data_has_been_invalidated |= ImGui::DragFloat("Height stretch", &_spectrum_height_scale, 0.01f, 0.0001f, FLT_MAX);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    });
}

} // namespace Cool