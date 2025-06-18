#pragma once
#include <mutex>
#include "Cool/File/File.h"
#include "Cool/Task/Task.hpp"
#include "Cool/Utils/Averager.h"

namespace Cool {

class Task_SaveVideoFrame : public Task {
public:
    Task_SaveVideoFrame(reg::AnyId const& owner_id, std::filesystem::path file_path, img::Image&& image, Averager<double>& average_save_time, std::mutex& average_save_time_mutex, std::atomic<int64_t>& nb_frames_which_finished_exporting, std::atomic<bool>& report_failure)
        : Task{fmt::format("Exporting video frame \"{}\"", Cool::File::file_name(file_path)), owner_id}
        , _file_path{std::move(file_path)}
        , _image{std::move(image)}
        , _average_save_time{&average_save_time}
        , _average_save_time_mutex{&average_save_time_mutex}
        , _nb_frames_which_finished_exporting{&nb_frames_which_finished_exporting}
        , _report_failure{&report_failure}
    {
    }

    auto progress() const -> float { return _progress.load(); }

private:
    auto execute() -> TaskCoroutine override;
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return true; }

private:
    std::filesystem::path     _file_path;
    std::optional<img::Image> _image;
    Averager<double>*         _average_save_time;
    std::mutex*               _average_save_time_mutex;
    std::atomic<int64_t>*     _nb_frames_which_finished_exporting;
    std::atomic<bool>*        _report_failure;
    std::atomic<float>        _progress{0.f};
};

} // namespace Cool
