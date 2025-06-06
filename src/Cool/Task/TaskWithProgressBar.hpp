#pragma once
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Task.hpp"

namespace Cool {

class TaskWithProgressBar : public Task {
public:
    TaskWithProgressBar(std::string name);

protected:
    void set_progress(float progress) { _data->progress.store(progress); }

    virtual auto notification_when_submitted() const -> ImGuiNotify::Notification { return notification_while_in_progress(); }
    auto         notification_while_in_progress() const -> ImGuiNotify::Notification; /// Can't be overriden, but you can override extra_imgui_below_progress_bar() to customize it
    virtual auto notification_after_execution_completes() const -> ImGuiNotify::Notification;

    virtual auto extra_imgui_below_progress_bar() const -> std::function<void()> // This function should capture everything by copy, it will live as long as the notification
    {
        return []() {};
    }

    void on_submit() override;
    void cleanup_impl(bool has_been_canceled) override;

    void change_notification(ImGuiNotify::Notification notification) { ImGuiNotify::change(_notification_id, std::move(notification)); }

private:
    ImGuiNotify::NotificationId _notification_id{};

    struct DataSharedWithNotification {
        std::atomic<float> progress{0.f};
    };
    std::shared_ptr<DataSharedWithNotification> _data{std::make_shared<DataSharedWithNotification>()}; // Needs to be a shared_ptr because the notification will hold on to it after we have been canceled, during its closing animation
};

} // namespace Cool