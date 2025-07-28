#pragma once
#include <mutex>
#include "Event.hpp"

namespace Cool {

inline auto response_queue() -> auto&
{
    static auto instance = std::vector<Event>{};
    return instance;
}

inline auto response_queue_mutex() -> auto&
{
    static auto instance = std::mutex{};
    return instance;
}

} // namespace Cool