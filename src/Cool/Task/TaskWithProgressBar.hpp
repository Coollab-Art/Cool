#pragma once
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Task.hpp"

namespace Cool {

class TaskWithProgressBar : public Task {
public:
    TaskWithProgressBar();

protected:
    auto cancel_requested() const -> bool { return _data->cancel.load(); }
    void set_progress(float progress) { _data->progress.store(progress); }

    virtual auto text_in_notification_while_waiting_to_execute() const -> std::string;
    virtual auto notification_after_execution_completes() const -> ImGuiNotify::Notification;
    virtual auto extra_imgui_below_progress_bar() const -> std::function<void()> // This function should capture everything by copy, it will live as long as the notification
    {
        return []() {
        };
    }

    void on_submit() override;
    void change_notification_when_execution_starts(); // Needs to be called by the children classes at the beginning of their execute()
    void cleanup(bool has_been_canceled) override;

private:
    auto is_quick_task() const -> bool override { return false; }
    void cancel() override { _data->cancel.store(true); }

private:
    ImGuiNotify::NotificationId _notification_id{};

    struct DataSharedWithNotification {
        std::atomic<bool>  cancel{false}; // Needs to be an atomic because it will be used on several threads (by do_work() and by cancel())
        std::atomic<float> progress{0.f};
    };
    std::shared_ptr<DataSharedWithNotification> _data{std::make_shared<DataSharedWithNotification>()}; // Needs to be a shared_ptr because the notification will hold on to it after we have been canceled, during its closing animation
};

} // namespace Cool