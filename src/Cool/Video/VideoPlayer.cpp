#include "VideoPlayer.h"
#include <exception>
#include <string>
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/TextureSource/default_textures.h"
#include "Cool/TextureSource/set_texture_from_image.h"
#include "Cool/Time/time_formatted_hms.h"
#include "hack_get_global_time_in_seconds.h"
#include "smart/smart.hpp"
#include "tl/expected.hpp"

namespace Cool {

// TODO(Video):
// cv::redirectError([](int status, const char* func_name,
//                      const char* err_msg, const char* file_name,
//                      int line, void* userdata) {
//     Cool::Log::ToUser::warning("OpenCV", err_msg);
//     return 1;
// });
// cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);

VideoPlayer::VideoPlayer(VideoDescriptor desc)
    : _desc{std::move(desc)}
{
    create_capture();
}

void VideoPlayer::create_capture()
{
    auto maybe = internal::CaptureState::create(path());
    if (maybe.has_value())
    {
        _capture_state = std::move(*maybe);
        _error_message.reset();
    }
    else
    {
        _capture_state.reset();
        _error_message = fmt::format("Failed to open video file {}:\n{}", path(), maybe.error());
    }
}

auto internal::CaptureState::create(std::filesystem::path const& path) -> tl::expected<internal::CaptureState, std::string>
{
    try
    {
        return internal::CaptureState{path};
    }
    catch (std::exception const& e)
    {
        return tl::make_unexpected(e.what());
    }
}

internal::CaptureState::CaptureState(std::filesystem::path const& path)
    : _capture{std::make_unique<ffmpeg::VideoDecoder>(path, AV_PIX_FMT_RGBA)}
{}

auto VideoPlayer::get_texture(Time time) -> Texture const*
{
    if (!_capture_state.has_value())
        return nullptr;

    _error_message.reset(); // Clear message from previous failure of get_texture(), we will re-add one if an error gets thrown again.
    try
    {
        return _capture_state->get_texture(time, settings(), path());
    }
    catch (std::exception const& e)
    {
        _error_message = e.what();
        return nullptr;
    }
}

auto VideoPlayer::get_current_texture() const -> Texture const*
{
    if (!_capture_state.has_value())
        return nullptr;
    auto const& maybe = _capture_state->get_current_texture();
    if (!maybe.has_value())
        return nullptr;
    return &*maybe;
}

auto VideoPlayer::detailed_video_info() const -> std::string const*
{
    if (!_capture_state.has_value())
        return nullptr;
    return &_capture_state->detailed_video_info();
}

auto internal::CaptureState::get_texture(Time time, VideoPlayerSettings const& settings, std::filesystem::path const& path) -> Texture const*
{
    // TODO(Video) Implement this to improve performance when playing video backward: https://www.opencv-srf.com/2017/12/play-video-file-backwards.html
    double time_in_seconds = (time - settings.start_time).as_seconds_double() * settings.playback_speed.value;
    switch (settings.loop_mode)
    {
    case VideoPlayerLoopMode::None:
    {
        if (time_in_seconds < 0. || time_in_seconds >= _capture->duration_in_seconds())
            return &transparent_texture();
        break;
    }
    case VideoPlayerLoopMode::Loop:
    {
        time_in_seconds = smart::mod(time_in_seconds, _capture->duration_in_seconds());
        break;
    }
    case VideoPlayerLoopMode::Hold:
    {
        time_in_seconds = std::clamp(time_in_seconds, 0., _capture->duration_in_seconds());
        break;
    }
    }
    // TODO(Video) debug option to log when seeking

    std::optional<ffmpeg::Frame> const maybe_frame = _capture->get_frame_at(time_in_seconds, hack_get_is_exporting() ? ffmpeg::SeekMode::Exact : ffmpeg::SeekMode::Fast);
    if (!maybe_frame) // Never happens unless the file is corrupted. Error message will be reported through ffmpeg::error_callback which will log to Log::ToUser::Warning
        return nullptr;

    set_texture_from_ffmpeg_image(_texture, *maybe_frame);
    if (DebugOptions::log_when_creating_textures())
        Log::info("Video File", fmt::format("Generated texture for {} at {}", path, time_formatted_hms(Time::seconds(time_in_seconds), true /*show_milliseconds*/)));

    return &*_texture;
}

} // namespace Cool