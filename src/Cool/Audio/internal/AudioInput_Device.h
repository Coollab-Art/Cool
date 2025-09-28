#pragma once
#include "Audio/Audio.hpp"
#include "IAudioInput.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace ser20 {

template<class Archive>
void serialize(Archive& archive, Audio::UseGivenDevice& device)
{
    archive(
        ser20::make_nvp("Name", device.name)
    );
}
template<class Archive>
void serialize(Archive&, Audio::UseDefaultDevice&)
{
}

} // namespace ser20

namespace Cool::internal {

class AudioInput_Device : public IAudioInput { // e.g. microphone
public:
    /// You MUST call this function at least once at the beginning to tell us the maximum numbers of samples you will query with `for_each_sample`.
    /// If that max number changes over time, you can call this function again to update it.
    void set_nb_of_retained_samples(size_t samples_count);

    void               for_each_audio_frame(int64_t frames_count, std::function<void(float)> const& callback) const override;
    [[nodiscard]] auto sample_rate() const -> float override;
    [[nodiscard]] auto is_playing() const -> bool override;

    void update() override;
    void start() override;
    void stop() override;

    auto               imgui(bool needs_to_highlight_error) -> bool override;
    [[nodiscard]] auto does_need_to_highlight_error() const -> bool override;

private:
    auto get_selected_input_device() const -> Audio::SelectedDevice;
    void set_selected_input_device(Audio::SelectedDevice);

private:
    float                       _volume{30.f};
    ImGuiNotify::NotificationId _notification_id{};
    std::string                 _last_logged_device_name{""};
    mutable bool                _has_started{false};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            ser20::make_nvp("Input Device", get_selected_input_device()),
            ser20::make_nvp("Volume", _volume)
        );
    }
    template<class Archive>
    void load(Archive& archive)
    {
        Audio::SelectedDevice device;
        archive(
            device,
            _volume
        );
        set_selected_input_device(std::move(device));
    }
};

} // namespace Cool::internal