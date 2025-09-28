#pragma once
#include "Cool/Utils/Stopwatch.hpp"

namespace Cool {

inline auto frame_time_stopwatch() -> Stopwatch&
{
    static auto instance = Stopwatch{};
    return instance;
}

} // namespace Cool