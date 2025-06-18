#pragma once
#include <chrono>

namespace Cool {

class BenchmarkScope {
public:
    explicit BenchmarkScope(std::string name, bool enabled);
    ~BenchmarkScope();

    BenchmarkScope(BenchmarkScope const&)                = delete;
    BenchmarkScope& operator=(BenchmarkScope const&)     = delete;
    BenchmarkScope(BenchmarkScope&&) noexcept            = delete;
    BenchmarkScope& operator=(BenchmarkScope&&) noexcept = delete;

private:
    bool _enabled;

    std::string                           _name;
    std::chrono::steady_clock::time_point _start_time;
};

#define BENCHMARK(name, enabled) BenchmarkScope _anon_var_##__COUNTER__ = BenchmarkScope{name, enabled};

} // namespace Cool