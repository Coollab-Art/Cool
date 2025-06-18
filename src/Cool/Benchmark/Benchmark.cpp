#include "Benchmark.hpp"
#include <chrono>
#include "Cool/Log/Log.hpp"
#include "Cool/Time/time_formatted_hms.h"

namespace Cool {

BenchmarkScope::BenchmarkScope(std::string name, bool enabled)
    : _enabled{enabled}
{
    if (!_enabled)
        return;

    _name       = std::move(name);
    _start_time = std::chrono::steady_clock::now();
}

BenchmarkScope::~BenchmarkScope()
{
    if (!_enabled)
        return;
#if DEBUG
    Cool::Log::warning("Benchmark", "You are in a DEBUG build, benchmark results are meaningless!");
#endif
    Cool::Log::info(
        "Benchmark",
        fmt::format(
            "\"{}\" took {}",
            _name,
            time_formatted_hms(std::chrono::steady_clock::now() - _start_time, true)
        )
    );
}

} // namespace Cool