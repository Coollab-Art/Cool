#pragma once
#include <chrono>
#include "Cool/Serialization/Json.hpp"
#include "TimeSpeed.h"

namespace Cool {

template<typename T>
concept StdChronoDuration = std::derived_from<T, std::chrono::duration<typename T::rep, typename T::period>>;

class Time {
public:
    Time() = default;
    Time(StdChronoDuration auto const& time) // NOLINT(*explicit*)
    {
        *this = Time::nanoseconds(std::chrono::nanoseconds{time}.count());
    }
    static auto nanoseconds(int64_t) -> Time;
    static auto seconds(double) -> Time;

    [[nodiscard]] auto as_seconds_double() const -> double;
    [[nodiscard]] auto as_seconds_float() const -> float;
    [[nodiscard]] auto as_nanoseconds_double() const -> double;
    [[nodiscard]] auto as_nanoseconds_float() const -> float;
    [[nodiscard]] auto as_nanoseconds_int64_t() const -> int64_t;

    friend auto operator+(Time, Time) -> Time;
    friend auto operator-(Time, Time) -> Time;
    void        operator+=(Time);
    void        operator-=(Time);

    friend auto operator*(Time, TimeSpeed) -> Time;
    friend auto operator*(TimeSpeed, Time) -> Time;

    friend auto operator<=>(Time, Time) = default;

private:
    int64_t _nanoseconds{0};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Nanoseconds", _nanoseconds)
        );
    }
};

inline void to_json(nlohmann::json& json, Time const& time)
{
    json_set(json, "Nanoseconds", time.as_nanoseconds_int64_t());
}

inline void from_json(nlohmann::json const& json, Time& time)
{
    int64_t nanoseconds{};
    json_get(json, "Nanoseconds", nanoseconds);
    time = Time::nanoseconds(json.at("Nanoseconds"));
}

struct imgui_drag_time_params {
    bool   show_milliseconds = true;
    float  drag_speed        = 0.01f;
    double min               = -DBL_MAX;
    double max               = +DBL_MAX;
};
auto imgui_drag_time(const char* label, Time* value, imgui_drag_time_params const& = {}) -> bool;
auto to_string(Time) -> std::string;

} // namespace Cool