// #pragma once
// #include <chrono>

// namespace Cool {

// struct BenchmarkEntry {
//     std::string                                          name;
//     std::chrono::steady_clock::time_point                start_time;
//     std::optional<std::chrono::steady_clock::time_point> stop_time;
// };

// class BenchmarkManager {
// public:
//     BenchmarkManager();

//     auto start_entry(std::string name) -> size_t;
//     void stop_entry(size_t);

// private:
//     std::chrono::steady_clock::time_point _origin_of_time;
//     size_t                                _next_index{0};
//     std::vector<BenchmarkEntry>           _entries{};
// };

// inline auto benchmark_manager() -> BenchmarkManager&
// {
//     static auto instance = BenchmarkManager{};
//     return instance;
// }

// } // namespace Cool