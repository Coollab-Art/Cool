#include "TaskWithProgressBar.hpp"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Task/TaskManager.hpp"
#include "reg/reg.hpp"

namespace Cool {

TaskWithProgressBar::TaskWithProgressBar(std::string name)
    : Task{std::move(name), reg::generate_uuid() /* give a unique id to this task, so that we can cancel it */}
{}

void TaskWithProgressBar::on_submit()
{
    _notification_id = ImGuiNotify::send(notification_when_submitted());
}

auto TaskWithProgressBar::notification_while_in_progress() const -> ImGuiNotify::Notification
{
    return ImGuiNotify::Notification{
        .type                 = ImGuiNotify::Type::Info,
        .title                = name(),
        .custom_imgui_content = [data = _data, task_id = owner_id(), extra_imgui = extra_imgui_below_progress_bar()](auto&&) {
            ImGuiExtras::progress_bar(data->progress.load());
            extra_imgui();
            if (ImGui::Button("Cancel"))
                task_manager().cancel_all(task_id);
        },
        .duration = std::nullopt,
        .closable = false,
    };
}

auto TaskWithProgressBar::notification_after_execution_completes() const -> ImGuiNotify::Notification
{
    return {
        .type     = ImGuiNotify::Type::Success,
        .title    = name(),
        .content  = "Completed",
        .duration = 2s,
    };
}

void TaskWithProgressBar::cleanup_impl(bool has_been_canceled)
{
    if (has_been_canceled)
        ImGuiNotify::close_immediately(_notification_id);
    else
        ImGuiNotify::change(_notification_id, notification_after_execution_completes());
}

} // namespace Cool