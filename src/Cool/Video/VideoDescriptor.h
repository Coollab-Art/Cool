#pragma once
#include "Cool/Path/Path.h"
#include "Cool/Time/Time.hpp"
#include "Cool/Time/TimeSpeed.h"

namespace Cool {

enum class VideoPlayerLoopMode {
    None,
    Loop,
    Hold,
    // TODO(Video) Mode Bounce (like mirror repeat for textures). But requires us to support playing videos backward
};

struct VideoPlayerSettings {
    VideoPlayerLoopMode loop_mode{VideoPlayerLoopMode::Loop};
    TimeSpeed           playback_speed{1.};
    Time                start_time{0s};

    friend auto operator==(VideoPlayerSettings const&, VideoPlayerSettings const&) -> bool = default;
    auto        imgui_widget() -> bool;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Loop mode", loop_mode),
            ser20::make_nvp("Playback speed", playback_speed),
            ser20::make_nvp("Start time", start_time)
        );
    }
};

struct VideoDescriptor {
    std::filesystem::path path{Cool::Path::default_video()};
    VideoPlayerSettings   settings{};

    auto imgui_widget() -> bool;

    friend auto operator==(VideoDescriptor const&, VideoDescriptor const&) -> bool = default;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("File Path", path),
            ser20::make_nvp("Settings", settings)
        );
    }
};

} // namespace Cool