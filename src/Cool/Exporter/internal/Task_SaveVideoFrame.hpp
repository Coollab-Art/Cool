#pragma once
#include <mutex>
#include "Cool/File/File.h"
#include "Cool/Task/Task.hpp"
#include "Cool/Utils/Averager.h"

namespace Cool {

class Task_SaveVideoFrame : public Task {
public:
    Task_SaveVideoFrame(reg::AnyId const& owner_id, std::filesystem::path file_path, img::Image&& image, Averager<double>& average_export_time, std::mutex& average_export_time_mutex, std::atomic<int64_t>& nb_frames_which_finished_exporting, std::atomic<bool>& report_failure)
        : Task{owner_id}
        , _file_path{std::move(file_path)}
        , _image{std::move(image)}
        , _average_export_time{&average_export_time}
        , _average_export_time_mutex{&average_export_time_mutex}
        , _nb_frames_which_finished_exporting{&nb_frames_which_finished_exporting}
        , _report_failure{&report_failure}
    {
    }

    auto name() const -> std::string override { return fmt::format("Exporting video frame \"{}\"", Cool::File::file_name(_file_path)); }

private:
    void execute() override;
    auto is_quick_task() const -> bool override { return false; }
    void cancel() override { _cancel.store(true); }
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return true; }

private:
    std::filesystem::path     _file_path;
    std::optional<img::Image> _image;
    Averager<double>*         _average_export_time;
    std::mutex*               _average_export_time_mutex;
    std::atomic<int64_t>*     _nb_frames_which_finished_exporting;
    std::atomic<bool>*        _report_failure;

    std::atomic<bool> _cancel{false};
};

} // namespace Cool
