#include "AudioInput_Device.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool::internal {

static auto input_stream() -> Audio::InputStream&
{
    static Audio::InputStream instance{
        [](RtAudioErrorType /* type */, std::string const& /* error_message */) {
            // Ignore all warnings, they tend to be pretty useless.
        }
    };
    return instance;
}

void AudioInput_Device::set_nb_of_retained_samples(size_t samples_count)
{
    input_stream().set_nb_of_retained_samples(samples_count);
}

void AudioInput_Device::for_each_audio_frame(int64_t frames_count, std::function<void(float)> const& callback) const
{
    _has_started = true;
    input_stream().for_each_sample(frames_count, [&](float const sample) { // We know that an input stream has only 1 channel, so 1 sample == 1 frame, no need to average across all the channels.
        callback(sample * _volume);
    });
}

auto AudioInput_Device::sample_rate() const -> float
{
    return static_cast<float>(input_stream().sample_rate());
}

auto AudioInput_Device::is_playing() const -> bool
{
    return input_stream().current_device_is_valid();
}

static auto get_device_name_impl(Audio::UseDefaultDevice) -> std::string
{
    auto const name = input_stream().device_info(input_stream().default_device_id()).name;
    return fmt::format("Default device: {}", name.empty() ? "NO DEFAULT DEVICE AVAILABLE" : name);
}
static auto get_device_name_impl(Audio::UseGivenDevice const& device) -> std::string
{
    return device.name;
}

static auto get_device_name(Audio::SelectedDevice const& selected_device) -> std::string
{
    return std::visit([](auto&& selected_device) { return get_device_name_impl(selected_device); }, selected_device);
}

void AudioInput_Device::update()
{
    if (!_has_started) // HACK don't update until for_each_audio_frame() has been called at least once, to make sure we don't create an audio stream and request access to the mic unless it is actually necessary. Otherwise we would use the mic as soon as the app starts up, even if we are not using audio nodes or showing the audio config window.
        return;
    input_stream().update();
    if (!input_stream().current_device_is_valid())
    {
        auto const device_name = get_device_name(input_stream().current_device());
        if (_last_logged_device_name != device_name) // Avoid spamming the log file every frame
        {
            _last_logged_device_name = device_name;
            ImGuiNotify::send_or_change(
                _notification_id,
                {
                    .type     = ImGuiNotify::Type::Error,
                    .title    = "Audio Device",
                    .content  = fmt::format("Invalid device \"{}\"", device_name),
                    .duration = std::nullopt,
                    .closable = false,
                }
            );
        }
    }
    else
    {
        ImGuiNotify::close_immediately(_notification_id);
        _last_logged_device_name = "";
    }
}

void AudioInput_Device::start()
{
    // No need to do anything on start, `input_stream().update()` will start the stream when necessary.
}

void AudioInput_Device::stop()
{
    input_stream().close();
    _has_started = false;
    ImGuiNotify::close_immediately(_notification_id);
    _last_logged_device_name = "";
}

auto AudioInput_Device::does_need_to_highlight_error() const -> bool
{
    return ImGuiNotify::is_notification_hovered(_notification_id);
}

auto AudioInput_Device::imgui(bool needs_to_highlight_error) -> bool
{
    bool b = false;

    auto const ids = input_stream().device_ids();
    Cool::ImGuiExtras::bring_attention_if(
        needs_to_highlight_error,
        [&]() {
            auto const combo_text = get_device_name(input_stream().current_device());
            if (ImGui::BeginCombo("Input device", combo_text.c_str()))
            {
                bool const is_selected = std::holds_alternative<Audio::UseDefaultDevice>(input_stream().current_device());
                if (ImGui::Selectable(get_device_name_impl(Audio::UseDefaultDevice{}).c_str(), is_selected))
                {
                    input_stream().use_default_device();
                    b = true;
                }
                if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    ImGui::SetItemDefaultFocus();

                for (auto const id : ids)
                {
                    auto const info         = input_stream().device_info(id);
                    bool const is_selected2 = info.name == combo_text;
                    if (ImGui::Selectable(info.name.c_str(), is_selected2))
                    {
                        input_stream().use_given_device(info);
                        b = true;
                    }

                    if (is_selected2) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
    );
    b |= ImGui::DragFloat("Volume Amplifier", &_volume, 0.03f, 0.f, 1000000.f);
    return b;
}

auto AudioInput_Device::get_selected_input_device() const -> Audio::SelectedDevice
{
    return input_stream().current_device();
}

void AudioInput_Device::set_selected_input_device(Audio::SelectedDevice device)
{
    input_stream().use_device(std::move(device));
}

} // namespace Cool::internal