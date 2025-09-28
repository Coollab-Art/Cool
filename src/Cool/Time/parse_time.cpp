#include "parse_time.h"
#include "Cool/String/String.h"

namespace Cool {

static auto is_num(char c) -> bool
{
    return ('0' <= c && c <= '9')
           || c == '.'
           || c == '+'
           || c == '-';
}
static auto is_whitespace(char c) -> bool
{
    return c == ' '
           || c == '\t'
           || c == '\n'
           || c == '\r';
}

namespace {
enum class Duration : std::uint8_t {
    Microsecond,
    Millisecond,
    Second,
    Minute,
    Hour,
    Day,
    Week,
    Month,
    Year,
};
} // namespace

static auto seconds_in(Duration duration) -> float
{
    using enum Duration;
    switch (duration)
    {
    case Microsecond:
        return 1.f / 1000.f / 1000.f;
    case Millisecond:
        return 1.f / 1000.f;
    case Second:
        return 1.f;
    case Minute:
        return 60.f;
    case Hour:
        return 3600.f;
    case Day:
        return 24.f * 3600.f;
    case Week:
        return 7.f * 24.f * 3600.f;
    case Month:
        return seconds_in(Year) / 12.f;
    case Year:
        return 365.25f * 24.f * 3600.f;
    }
    assert(false);
    return 1.f;
}

static auto seconds_in(std::string str, Duration& implicit_duration) -> float
{
    str = Cool::String::to_lower(str);

    if (str == "ms"
        || str == "milli"
        || str == "millis"
        || str == "millisecond"
        || str == "milliseconds")
    {
        implicit_duration = Duration::Microsecond;
        return seconds_in(Duration::Millisecond);
    }
    if (str == "s"
        || str == "second"
        || str == "seconds")
    {
        implicit_duration = Duration::Millisecond;
        return seconds_in(Duration::Second);
    }
    if (str == "m"
        || str == "min"
        || str == "minute"
        || str == "minutes")
    {
        implicit_duration = Duration::Second;
        return seconds_in(Duration::Minute);
    }
    if (str == "h"
        || str == "hour"
        || str == "hours")
    {
        implicit_duration = Duration::Minute;
        return seconds_in(Duration::Hour);
    }
    if (str == "d"
        || str == "day"
        || str == "days")
    {
        implicit_duration = Duration::Hour;
        return seconds_in(Duration::Day);
    }
    if (str == "w"
        || str == "week"
        || str == "weeks")
    {
        implicit_duration = Duration::Day;
        return seconds_in(Duration::Week);
    }
    if (str == "month"
        || str == "months")
    {
        implicit_duration = Duration::Day;
        return seconds_in(Duration::Month);
    }
    if (str == "y"
        || str == "year"
        || str == "years")
    {
        implicit_duration = Duration::Month;
        return seconds_in(Duration::Year);
    }
    return seconds_in(implicit_duration);
}

/// Returns time in seconds
auto parse_time(const char* str) -> float
{
    float    total_time{0.f};
    Duration implicit_duration{Duration::Second};
    while (*str != 0)
    {
        // First, read the string to get a number
        float const current_val = [&]() { // IIFE
            std::string number_token{};
            for (; *str != 0 && (is_num(*str) || is_whitespace(*str)); str++) // NOLINT(*pointer-arithmetic)
                number_token += *str;
            try
            {
                return std::stof(number_token);
            }
            catch (...)
            {
                return 0.f;
            }
        }();
        { // Then, read the string to get a word (minute / hour / etc.)
            std::string work_token{};
            for (; *str != 0 && !is_num(*str); str++) // NOLINT(*pointer-arithmetic)
            {
                if (!is_whitespace(*str))
                    work_token += *str;
            }
            total_time += current_val * seconds_in(work_token, implicit_duration);
        }
    }
    return total_time;
}

} // namespace Cool

#if COOL_ENABLE_TESTS
#include <doctest/doctest.h>
TEST_CASE("parse_time()")
{
    CHECK(doctest::Approx{Cool::parse_time("3m 2 s")} == 3 * 60 + 2);
    CHECK(doctest::Approx{Cool::parse_time("3m-2s")} == 3 * 60 - 2);
    CHECK(doctest::Approx{Cool::parse_time("3.78m")} == 3.78f * 60.f);
    // clang-format off
    CHECK(doctest::Approx{Cool::parse_time("4w 8d 13h 47s 318ms")} == 4 * 7 * 24 * 3600
                                                                    + 8     * 24 * 3600
                                                                    + 13         * 3600
                                                                    + 47
                                                                    + 318.f /1000.f);
    // clang-format on

    CHECK(doctest::Approx{Cool::parse_time("999m")} == 999 * 60);
    CHECK(doctest::Approx{Cool::parse_time("999 m")} == 999 * 60);
    CHECK(doctest::Approx{Cool::parse_time("999  m")} == 999 * 60);
    CHECK(doctest::Approx{Cool::parse_time("  999    m   ")} == 999 * 60);
    CHECK(doctest::Approx{Cool::parse_time("+999m")} == 999 * 60);
    CHECK(doctest::Approx{Cool::parse_time("-999m")} == -999 * 60);

    CHECK(doctest::Approx{Cool::parse_time("1ms")} == 1.f / 1000.f);
    CHECK(doctest::Approx{Cool::parse_time("1milli")} == 1.f / 1000.f);
    CHECK(doctest::Approx{Cool::parse_time("1millis")} == 1.f / 1000.f);
    CHECK(doctest::Approx{Cool::parse_time("1millisecond")} == 1.f / 1000.f);
    CHECK(doctest::Approx{Cool::parse_time("1milliseconds")} == 1.f / 1000.f);

    CHECK(doctest::Approx{Cool::parse_time("1s")} == 1);
    CHECK(doctest::Approx{Cool::parse_time("1sec")} == 1);
    CHECK(doctest::Approx{Cool::parse_time("1second")} == 1);
    CHECK(doctest::Approx{Cool::parse_time("1seconds")} == 1);

    CHECK(doctest::Approx{Cool::parse_time("1m")} == 60);
    CHECK(doctest::Approx{Cool::parse_time("1min")} == 60);
    CHECK(doctest::Approx{Cool::parse_time("1minute")} == 60);
    CHECK(doctest::Approx{Cool::parse_time("1minutes")} == 60);

    CHECK(doctest::Approx{Cool::parse_time("1h")} == 3600);
    CHECK(doctest::Approx{Cool::parse_time("1hour")} == 3600);
    CHECK(doctest::Approx{Cool::parse_time("1hours")} == 3600);

    CHECK(doctest::Approx{Cool::parse_time("1d")} == 24 * 3600);
    CHECK(doctest::Approx{Cool::parse_time("1day")} == 24 * 3600);
    CHECK(doctest::Approx{Cool::parse_time("1days")} == 24 * 3600);

    CHECK(doctest::Approx{Cool::parse_time("1w")} == 7 * 24 * 3600);
    CHECK(doctest::Approx{Cool::parse_time("1week")} == 7 * 24 * 3600);
    CHECK(doctest::Approx{Cool::parse_time("1weeks")} == 7 * 24 * 3600);
}
#endif