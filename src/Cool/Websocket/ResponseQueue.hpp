#pragma once
#include <mutex>
#include "Response.hpp"

namespace Cool {

inline auto response_queue() -> auto&
{
    static auto instance = std::vector<Response>{};
    return instance;
}

inline auto response_queue_mutex() -> auto&
{
    static auto instance = std::mutex{};
    return instance;
}

} // namespace Cool