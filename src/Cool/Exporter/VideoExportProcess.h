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
    auto estimated_remaining_time() -> Time;

private:
    std::filesystem::path _folder_path;
    img::Size             _size;
    Clock_FixedTimestep   _clock;

    bool              _should_stop_asap{false};
    std::atomic<bool> _failure_has_been_reported{false};

    int64_t              _nb_frames_rendered{0};
    std::atomic<int64_t> _nb_frames_saved{0};
    int64_t              _total_nb_of_frames_in_sequence;
    int64_t              _next_frame_number;

    reg::AnyId _tasks_owner_id{reg::generate_uuid()};

    std::list<std::shared_ptr<Task_SaveVideoFrame>> _tasks_in_progress;
    std::list<std::shared_ptr<Task_SaveVideoFrame>> _next_tasks;

    Averager<double> _average_render_time{};
    Averager<double> _average_save_time{};
    std::mutex       _average_save_time_mutex{};

    no_sleep::Scoped _disable_sleep{COOL_APP_NAME, COOL_APP_NAME " is exporting a video", no_sleep::Mode::ScreenCanTurnOffButKeepComputing};
};

} // namespace Cool
