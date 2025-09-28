#pragma once

namespace Cool {

class Delay {
public:
    explicit Delay(std::chrono::milliseconds delay)
        : _end_time{std::chrono::steady_clock::now() + delay}
    {}

    auto has_expired() const -> bool { return std::chrono::steady_clock::now() > _end_time; }

private:
    std::chrono::steady_clock::time_point _end_time;
};

} // namespace Cool