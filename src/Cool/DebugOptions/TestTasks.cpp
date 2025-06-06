#include "TestTasks.hpp"
#include <imgui.h>
#include <ImGuiNotify/ImGuiNotify.hpp>
#include <chrono>
#include <memory>
#include <reg/src/generate_uuid.hpp>
#include <string>
#include "Cool/Benchmark/Benchmark.hpp"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/Task/TaskManager.hpp"
#include "Cool/Task/TaskWithProgressBar.hpp"

namespace Cool {

namespace {

auto next_id() -> int&
{
    static int instance{1};
    return instance;
}

class Task_LongProcess : public TaskWithProgressBar {
public:
    explicit Task_LongProcess(int iterations_count)
        : TaskWithProgressBar{fmt::format("Long Process {}", next_id()++)}
        , _iterations_count{iterations_count}
    {}

private:
    auto execute() -> TaskCoroutine override
    {
        for (int i = 0; i < _iterations_count; ++i)
        {
            if (has_been_canceled())
                break;
            set_progress(static_cast<float>(i) / static_cast<float>(_iterations_count));

            std::this_thread::sleep_for(10ms);
            co_await SuspendTask{};
        }
        co_return;
    }

    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return true; }

private:
    int _iterations_count;

    BENCHMARK(name(), DebugOptions::benchmark_test_tasks());
};

class Task_SayHello : public Task {
public:
    explicit Task_SayHello(bool loop = false)
        : Task{fmt::format("Say Hello {}", next_id()++)}
        , _loop{loop}
    {}

private:
    auto execute() -> TaskCoroutine override
    {
        ImGuiNotify::send({
            .type    = _loop ? ImGuiNotify::Type::Warning : ImGuiNotify::Type::Success,
            .title   = name(),
            .content = fmt::format("Hello from {} ago", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start_time)),
        });
        if (_loop)
            task_manager().submit(after(2s), std::make_shared<Task_SayHello>(true));
        co_return;
    }

    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return false; }

private:
    bool _loop{};

    std::chrono::steady_clock::time_point _start_time{std::chrono::steady_clock::now()};
};

} // namespace

void TestTasks::imgui()
{
    if (ImGui::Button("Long Task"))
        task_manager().submit(std::make_shared<Task_LongProcess>(1000));
    if (ImGui::Button("Medium Task"))
        task_manager().submit(std::make_shared<Task_LongProcess>(100));
    if (ImGui::Button("Short Task"))
        task_manager().submit(std::make_shared<Task_LongProcess>(3));
    if (ImGui::Button("Run in 2 seconds"))
        task_manager().submit(after(2s), std::make_shared<Task_SayHello>());
    if (ImGui::Button("Loop every 2 seconds"))
        task_manager().submit(std::make_shared<Task_SayHello>(true));

    ImGui::NewLine();
    task_manager().imgui_show_debug_tasks_list();
}

} // namespace Cool
