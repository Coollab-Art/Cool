#include "TaskWithProgressBar.hpp"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Task/TaskManager.hpp"
#include "reg/reg.hpp"

namespace Cool {

TaskWithProgressBar::TaskWithProgressBar()
    : Task{reg::generate_uuid() /* give a unique id to this task, so that we can cancel it */}
{}

void TaskWithProgressBar::on_submit()
{
    _notification_id = ImGuiNotify::send({
        .type                 = ImGuiNotify::Type::Info,
        .title                = name(),
        .content              = text_in_notification_while_waiting_to_execute(),
        .custom_imgui_content = [data = _data, task_id = owner_id()]() {
            ImGuiExtras::disabled_if(data->cancel.load(), "", [&]() {
                if (ImGui::Button("Cancel"))
                    task_manager().cancel_all(task_id);
            });
        },
        .duration = std::nullopt,
        .closable = false,
    });
}

void TaskWithProgressBar::change_notification_when_execution_starts()
{
    ImGuiNotify::change(
        _notification_id,
        {
            .type                 = ImGuiNotify::Type::Info,
            .title                = name(),
            .custom_imgui_content = [data = _data, task_id = owner_id(), extra_imgui = extra_imgui_below_progress_bar()]() {
                ImGuiExtras::disabled_if(data->cancel.load(), "", [&]() {
                    ImGuiExtras::progress_bar(data->progress.load());
                    extra_imgui();
                    if (ImGui::Button("Cancel"))
                        task_manager().cancel_all(task_id);
                });
            },
            .duration = std::nullopt,
            .closable = false,
        }
    );
}

auto TaskWithProgressBar::text_in_notification_while_waiting_to_execute() const -> std::string
{
    return "Waiting for other tasks to finish before we can start this one";
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

void TaskWithProgressBar::cleanup(bool has_been_canceled)
{
    if (has_been_canceled)
        ImGuiNotify::close_immediately(_notification_id);
    else
        ImGuiNotify::change(_notification_id, notification_after_execution_completes());
}

} // namespace Cool