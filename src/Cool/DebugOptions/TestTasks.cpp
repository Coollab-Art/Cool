#include "TestTasks.hpp"
#include <imgui.h>
#include <ImGuiNotify/ImGuiNotify.hpp>
#include <chrono>
#include <memory>
#include <reg/src/generate_uuid.hpp>
#include <string>
#include "Cool/Task/TaskManager.hpp"
#include "Cool/Task/TaskWithProgressBar.hpp"

namespace Cool {

namespace {

auto next_id() -> int&
{
    static int instance{0};
    return instance;
}

class Task_LongProcess : public TaskWithProgressBar {
public:
    explicit Task_LongProcess(int count_max)
        : _count_max{count_max}
    {}

    auto name() const -> std::string override { return fmt::format("Long Process {}", _id); }

private:
    void execute() override
    {
        TaskWithProgressBar::change_notification_when_execution_starts();

        int count{0};
        for (int _ = 0; _ < _count_max; ++_)
        {
            count++;

            if (cancel_requested())
                return;
            set_progress(static_cast<float>(count) / static_cast<float>(_count_max));
        }
    }

    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return true; }

private:
    int _id{next_id()++};
    int _count_max;
};

class Task_SayHello : public Task {
public:
    explicit Task_SayHello(bool loop = false)
        : _loop{loop}
    {}

    auto name() const -> std::string override { return fmt::format("Say Hello {}", _id); }

private:
    void execute() override
    {
        ImGuiNotify::send({
            .type    = _loop ? ImGuiNotify::Type::Warning : ImGuiNotify::Type::Success,
            .title   = name(),
            .content = fmt::format("Hello from {} ago", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start_time)),
        });
        if (_loop)
            task_manager().submit(after(2s), std::make_shared<Task_SayHello>(true));
    }

    auto is_quick_task() const -> bool override { return true; }
    void cancel() override {}
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return false; }

private:
    int  _id{next_id()++};
    bool _loop{};

    std::chrono::steady_clock::time_point _start_time{std::chrono::steady_clock::now()};
};

} // namespace

void TestTasks::imgui()
{
    ImGui::TextUnformatted(fmt::format("Waiting for condition: {}", task_manager().num_tasks_waiting_for_condition()).c_str());
    ImGui::TextUnformatted(fmt::format("Waiting for thread: {}", task_manager().num_tasks_waiting_for_thread()).c_str());
    ImGui::TextUnformatted(fmt::format("Processing: {}", task_manager().num_tasks_processing()).c_str());

    if (ImGui::Button("Long Task"))
        task_manager().submit(std::make_shared<Task_LongProcess>(1000000000));
    if (ImGui::Button("Medium Task"))
        task_manager().submit(std::make_shared<Task_LongProcess>(100000000));
    if (ImGui::Button("Short Task"))
        task_manager().submit(std::make_shared<Task_LongProcess>(10000000));
    if (ImGui::Button("Run in 2 seconds"))
        task_manager().submit(after(2s), std::make_shared<Task_SayHello>());
    if (ImGui::Button("Loop every 2 seconds"))
        task_manager().submit(std::make_shared<Task_SayHello>(true));

    ImGui::NewLine();
    if (ImGui::Button("Cancel all tasks"))
        task_manager().cancel_all();
}

} // namespace Cool
