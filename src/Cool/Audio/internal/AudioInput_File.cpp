#include "AudioInput_File.h"
#include <optional>
#include "Audio/Audio.hpp"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/NfdFileFilter/NfdFileFilter.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool::internal {

void AudioInput_File::for_each_audio_frame(int64_t frames_count, std::function<void(float)> const& callback) const
{
    for (int i = 0; i < frames_count; ++i)
        callback(Audio::player().sample_unaltered_volume(i - frames_count / 2 + Audio::player().current_frame_index()));
}

auto AudioInput_File::sample_rate() const -> float
{
    return static_cast<float>(Audio::player().audio_data().sample_rate);
}

auto AudioInput_File::is_playing() const -> bool
{
    return Audio::player().is_playing();
}

void AudioInput_File::update()
{
    Audio::player().properties() = _properties;
    Audio::player().properties().is_muted |= _force_mute;
}

void AudioInput_File::start()
{
    if (_path != _currently_loaded_path)
        try_load_current_file();
}

void AudioInput_File::stop()
{
    Audio::player().pause(); // Stop the playing of the audio
    ImGuiNotify::close_immediately(_notification_id);
}

auto AudioInput_File::does_need_to_highlight_error() const -> bool
{
    return ImGuiNotify::is_notification_hovered(_notification_id);
}

static auto imgui_widgets(Audio::PlayerProperties& props) -> bool
{
    bool b = false; // b is not affected when we change properties related to the volume, because we ignore these properties when computing the features like volume, fft etc.
    b |= ImGuiExtras::checkbox_button(ICOMOON_LOOP, &props.does_loop);
    ImGui::SetItemTooltip("%s", props.does_loop ? "The audio will loop." : "The audio will not loop. It will only play when the time is between 0 and the duration of the audio.");

    ImGui::SameLine();

    if (ImGuiExtras::button_with_text_icon(props.is_muted ? ICOMOON_VOLUME_MUTE2 : ICOMOON_VOLUME_MEDIUM))
        props.is_muted = !props.is_muted;
    ImGui::SetItemTooltip("%s", props.is_muted ? "Muted" : "Not muted");

    ImGui::SameLine();

    ImGuiExtras::disabled_if(props.is_muted, "The audio is muted.", [&]() {
        ImGui::SliderFloat("Playback Volume", &props.volume, 0.f, 1.f);
    });

    return b;
}

auto AudioInput_File::imgui(bool needs_to_highlight_error) -> bool
{
    bool b = false;
    Cool::ImGuiExtras::bring_attention_if(
        needs_to_highlight_error,
        [&]() {
            if (ImGuiExtras::file_opening("Audio file", &_path, NfdFileFilter::Audio))
            {
                try_load_current_file();
                b = true;
            }
        }
    );
    b |= imgui_widgets(_properties);
    return b;
}

void AudioInput_File::try_load_current_file()
{
    if (_path.empty())
    {
        ImGuiNotify::close_immediately(_notification_id);
        Audio::player().reset_audio_data();
        _currently_loaded_path = "";
        return;
    }

    try
    {
        load_audio_file(Audio::player(), _path);
        _currently_loaded_path = _path;
        ImGuiNotify::close_immediately(_notification_id);
    }
    catch (std::exception& e)
    {
        ImGuiNotify::send_or_change(
            _notification_id,
            {
                .type     = ImGuiNotify::Type::Error,
                .title    = "Audio File",
                .content  = e.what(),
                .duration = std::nullopt,
                .closable = false,
            }
        );
        Audio::player().reset_audio_data();
        _currently_loaded_path = "";
    }
}

} // namespace Cool::internal