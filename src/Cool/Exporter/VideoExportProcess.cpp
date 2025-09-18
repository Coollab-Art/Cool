#include "VideoExportProcess.h"
#include <chrono>
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/String/String.h"
#include "Cool/Task/TaskManager.hpp"
#include "Cool/Time/time_formatted_hms.h"
#include "ExporterU.h"
#include "internal/ORIGIN_OF_FRAMES.hpp"
#include "internal/Task_SaveVideoFrame.hpp"

namespace Cool {

static auto nb_digits(int64_t n) -> int
{
    return static_cast<int>(std::ceil(std::log10(n)));
}

VideoExportProcess::VideoExportProcess(VideoExportParams const& params, TimeSpeed time_speed, std::filesystem::path const& folder_path, img::Size size)
    : _folder_path{folder_path}
    , _size{size}
    , _clock{params.fps}
    , _total_nb_of_frames_in_sequence{static_cast<int64_t>(std::ceil((params.end - params.beginning).as_seconds_double() * params.fps))}
    , _next_frame_number{ORIGIN_OF_FRAMES + static_cast<int64_t>(std::ceil(params.beginning.as_seconds_double() * params.fps))} // Makes sure than if we export frames from 0 to 10 seconds, and then decide to extend that video and export frames from 10 to 20 seconds, that second batch of frames will have numbers that follow the ones of the first batch, allowing us to create a unified image sequence with numbers that match up.
{
    _clock.set_time(params.beginning, true /* force_delta_time_to_ignore_the_change */);
    _clock.time_speed().value() = time_speed;
}

bool VideoExportProcess::update(Polaroid const& polaroid)
{
    if (_failure_has_been_reported.load())
        return true; // Abort the export

    if (_should_stop_asap)
    {
        ImGuiNotify::send(ExporterU::notification_after_video_export_canceled(_folder_path));
        task_manager().cancel_all(_tasks_owner_id);
        return true; // The export has been canceled
    }

    if (_nb_frames_saved.load() == _total_nb_of_frames_in_sequence)
    {
        ImGuiNotify::send(ExporterU::notification_after_video_export_success(_folder_path));
        return true; // The export is finished
    }

    std::erase_if(_tasks_in_progress, [](auto const& task) {
        return task->has_been_executed() || task->has_been_canceled();
    });

    // Submit tasks that were waiting
    while (_tasks_in_progress.size() < task_manager().threads_count() && !_next_tasks.empty())
    {
        task_manager().submit(_next_tasks.front());
        _tasks_in_progress.emplace_back(std::move(_next_tasks.front()));
        _next_tasks.pop_front();
    }

    // Render one more frame if necessary

    if (_nb_frames_rendered == _total_nb_of_frames_in_sequence)
        return false; // The export is not finished but we have already rendered all the frames and just need to wait for save to be saved as images on disk

    if (_next_tasks.size() >= task_manager().threads_count())
        return false; // The export is not finished but the thread pool is already saturated, there is no point in preparing more tasks right now

    // Render one frame
    {
        auto const start = std::chrono::steady_clock::now();
        polaroid.render(_size, _clock.time(), _clock.delta_time());
        auto const end = std::chrono::steady_clock::now();
        _average_render_time.push(Time{end - start}.as_seconds_double());
    }
    _nb_frames_rendered++;

    _next_tasks.emplace_back(std::make_shared<Task_SaveVideoFrame>(
        _tasks_owner_id,
        (_folder_path / String::to_string(_next_frame_number, nb_digits(ORIGIN_OF_FRAMES))).replace_extension("png"),
        polaroid.texture().download_pixels(),
        _average_save_time,
        _average_save_time_mutex,
        _nb_frames_saved,
        _failure_has_been_reported
    ));
    _next_frame_number++;
    _clock.update();

    return false;
}

auto VideoExportProcess::estimated_remaining_time() -> Time
{
    double const nb_frames_to_render = static_cast<double>(_total_nb_of_frames_in_sequence - _nb_frames_rendered); // NOLINT(*use-auto)
    double       nb_frames_to_save   = static_cast<double>(_total_nb_of_frames_in_sequence - _nb_frames_saved);    // NOLINT(*use-auto)

    for (auto const& task : _tasks_in_progress)
    {
        if (!task->has_been_executed())
            nb_frames_to_save -= task->progress();
    }

    return Time::seconds(
        nb_frames_to_render * _average_render_time
        + nb_frames_to_save * _average_save_time / static_cast<double>(task_manager().threads_count())
        + 1.
    );
}

void VideoExportProcess::imgui(std::function<void()> const& extra_widgets)
{
    auto const frame_count = _nb_frames_saved.load();

    float tasks_progress{0.f};
    for (auto const& task : _tasks_in_progress)
    {
        if (!task->has_been_executed())
            tasks_progress += task->progress();
    }

    // Progress bar
    float const progress = (static_cast<float>(frame_count) + tasks_progress) / static_cast<float>(_total_nb_of_frames_in_sequence);
    ImGuiExtras::progress_bar(progress);

    // Frames count
    ImGui::PushFont(Font::monospace());
    ImGui::TextUnformatted(
        fmt::format(
            "{} / {} frames",
            String::to_string(frame_count, nb_digits(_total_nb_of_frames_in_sequence)),
            _total_nb_of_frames_in_sequence
        )
            .c_str()
    );
    ImGui::TextUnformatted(time_formatted_hms(estimated_remaining_time()).c_str());
    ImGui::SameLine();
    ImGui::TextUnformatted("remaining");
    ImGui::PopFont();

    // Stop exporting
    if (ImGui::Button("Stop exporting"))
        _should_stop_asap = true;

    extra_widgets();
}

} // namespace Cool