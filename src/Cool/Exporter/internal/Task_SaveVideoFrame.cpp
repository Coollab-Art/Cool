#include "Task_SaveVideoFrame.hpp"
#include "../ExporterU.h"
#include "Cool/File/File.h"
#include "Cool/Image/SaveImage.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

auto Task_SaveVideoFrame::execute() -> TaskCoroutine
{
    auto const start = std::chrono::steady_clock::now();

    auto const result = ImageU::save(
        _file_path, *_image,
        {
            .cancel_requested = [&]() { return has_been_canceled(); },
            .set_progress     = [&](float progress) { _progress.store(progress); },
        }
    );
    if (!result.has_value())
    {
        if (has_been_canceled())
            co_return; // Don't log any error if we were just canceled
        bool const has_already_been_reported = _report_failure->exchange(true);
        if (!has_already_been_reported)
            ImGuiNotify::send(ExporterU::notification_after_video_export_failure(Cool::File::without_file_name(_file_path), result.error()));
        co_return;
    }

    auto const end = std::chrono::steady_clock::now();
    {
        auto lock = std::unique_lock{*_average_save_time_mutex};
        _average_save_time->push(Time{end - start}.as_seconds_double());
    }
    (*_nb_frames_which_finished_exporting)++;
}

} // namespace Cool
