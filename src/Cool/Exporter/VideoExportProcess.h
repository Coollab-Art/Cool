#pragma once
#include <mutex>
#include "Cool/Time/Clock_FixedTimestep.h"
#include "Cool/Utils/Averager.h"
#include "Polaroid.hpp"
#include "VideoExportParams.h"
#include "internal/Task_SaveVideoFrame.hpp"
#include "no_sleep/no_sleep.hpp"
#include "reg/reg.hpp"

namespace Cool {

class VideoExportProcess {
public:
    VideoExportProcess(VideoExportParams const& params, TimeSpeed time_speed, std::filesystem::path const& folder_path, img::Size size);
    auto update(Polaroid const& polaroid) -> bool;
    void imgui(std::function<void()> const& extra_widgets);

    auto clock() const -> Clock const& { return _clock; }

private:
    auto estimated_remaining_time() -> Time;
    void update_time_estimate();

private:
    std::filesystem::path _folder_path;
    img::Size             _size;
    Clock_FixedTimestep   _clock;

    int64_t              _nb_frames_sent_to_thread_pool{0};
    std::atomic<int64_t> _nb_frames_which_finished_exporting{0};
    int64_t              _total_nb_of_frames_in_sequence;
    // int64_t              _frame_numbering_offset;
    int64_t _next_frame_number;

    std::atomic<bool> _failure_has_been_reported{false};

    std::chrono::steady_clock::time_point _start_time{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point _last_render{};
    Averager<double>                      _average_time_between_two_renders{};
    Averager<double>                      _average_export_time{};
    std::mutex                            _average_export_time_mutex;

    no_sleep::Scoped _disable_sleep{COOL_APP_NAME, COOL_APP_NAME " is exporting a video", no_sleep::Mode::ScreenCanTurnOffButKeepComputing};

    bool       _should_stop_asap = false;
    reg::AnyId _tasks_owner_id{reg::generate_uuid()};

    std::list<std::shared_ptr<Task_SaveVideoFrame>> _tasks_in_progress;
    std::list<std::shared_ptr<Task_SaveVideoFrame>> _next_tasks;
};

} // namespace Cool
