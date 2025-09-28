#include "Stopwatch.hpp"
#include <numeric>
#include "Cool/ImGui/Fonts.h"
#include "imgui.h"

namespace Cool {

void Stopwatch::start()
{
    _start_time = std::chrono::steady_clock::now();
}

void Stopwatch::stop()
{
    if (!_start_time.has_value())
        return;

    auto const end_time = std::chrono::steady_clock::now();
    _times.push_back(static_cast<float>((end_time - *_start_time).count()) / 1'000'000.f);
    if (_times.size() > _samples_count)
        _times.erase(_times.begin());
}

auto Stopwatch::average_time_ms() const -> float
{
    auto const& times = _times_when_paused.empty() ? _times : _times_when_paused;
    return std::accumulate(times.begin(), times.end(), 0.f) / static_cast<float>(times.size());
}

void Stopwatch::imgui_plot()
{
    ImGui::PushFont(Cool::Font::monospace());

    auto const ms        = average_time_ms();
    auto const framerate = 1000.f / ms;

    ImGui::Text(framerate >= 100.f ? "%.1f FPS" : " %.1f FPS", framerate);
    ImGui::Text(ms >= 10.f ? "%.2f ms" : " %.2f ms", ms);

    ImGui::PopFont();

    auto const& times = _times_when_paused.empty() ? _times : _times_when_paused;
    ImGui::PlotLines("Timings (ms)", times.data(), static_cast<int>(times.size()), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2{0.f, 100.f});

    if (ImGui::Button(_times_when_paused.empty() ? "Pause" : "Play"))
    {
        if (_times_when_paused.empty())
            _times_when_paused = _times;
        else
            _times_when_paused.clear();
    }
    ImGui::SameLine();
    if (ImGui::InputScalar("Samples count", ImGuiDataType_U64, &_samples_count))
        _times.clear();
}

} // namespace Cool