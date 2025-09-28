#include "Task_SaveImage.hpp"
#include <open/open.hpp>
#include "Cool/Image/SaveImage.h"
#include "Cool/Task/TaskWithProgressBar.hpp"
#include "ExporterU.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

Task_SaveImage::Task_SaveImage(std::filesystem::path const& file_path, img::Image image)
    : _file_path{file_path}
    , _image{std::move(image)}
{
    File::mark_file_path_unavailable(file_path); // The file will not be created immediately, but we must know that it is already taken so that we don't try to create another image with the same path
}

void Task_SaveImage::execute()
{
    TaskWithProgressBar::change_notification_when_execution_starts();

    _result = ImageU::save(
        _file_path, _image,
        {
            .cancel_requested = [&]() { return cancel_requested(); },
            .set_progress     = [&](float progress) { set_progress(progress); },
        }
    );
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