#include "Task_SaveImage.hpp"
#include <mutex>
#include <open/open.hpp>
#include "Cool/Image/SaveImage.h"
#include "Cool/Task/TaskWithProgressBar.hpp"
#include "Cool/Websocket/Event.hpp"
#include "Cool/Websocket/ResponseQueue.hpp"
#include "ExporterU.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

Task_SaveImage::Task_SaveImage(std::filesystem::path file_path, img::Image image, std::function<void(Event)> _start_callback, std::function<void(Event)> _end_callback)
    : TaskWithProgressBar{fmt::format("Exporting image \"{}\"", Cool::File::file_name(file_path))}
    , _file_path{std::move(file_path)}
    , _image{std::move(image)}
    , _start_callback{std::move(_start_callback)}
    , _end_callback{std::move(_end_callback)}
{
    File::mark_file_path_unavailable(_file_path); // The file will not be created immediately, but we must know that it is already taken so that we don't try to create another image with the same path
}

auto Task_SaveImage::execute() -> TaskCoroutine
{
    // TODO(Task) pause the coroutine regularly
    {
        if (_start_callback)
        {
            auto lock = std::unique_lock{response_queue_mutex()};
            _start_callback(Event_ImageExportStarted{
                .size = _image.size(),
                .path = _file_path,
            });
        };
    }
    _result = ImageU::save(
        _file_path, _image,
        {
            .cancel_requested = [&]() { return has_been_canceled(); },
            .set_progress     = [&](float progress) { set_progress(progress); },
        }
    );
    {
        if (_end_callback)
        {
            auto lock = std::unique_lock{response_queue_mutex()};
            _end_callback(Event_ExportedImage{
                .size = _image.size(),
                .path = _file_path,
            });
        };
    }
    co_return;
}

auto Task_SaveImage::notification_after_execution_completes() const -> ImGuiNotify::Notification
{
    if (_result.has_value())
    {
        auto success_notification                 = TaskWithProgressBar::notification_after_execution_completes();
        success_notification.custom_imgui_content = [path = _file_path]() {
            if (ImGui::Button(fmt::format("Open in file explorer").c_str()))
                open_focused_in_explorer(path);
        };
        return success_notification;
    }
    else // NOLINT(*else-after-return)
    {
        return {
            .type    = ImGuiNotify::Type::Error,
            .title   = name(),
            .content = _result.error(),
        };
    }
}

} // namespace Cool