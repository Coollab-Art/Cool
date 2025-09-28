#pragma once

namespace Cool {

class Stopwatch {
public:
    void start();
    void stop();

    auto average_time_ms() const -> float;
    void imgui_plot();

private:
    std::optional<std::chrono::steady_clock::time_point> _start_time{};
    std::vector<float>                                   _times{};
    std::vector<float>                                   _times_when_paused{};
    size_t                                               _samples_count{300};
};

} // namespace Cool