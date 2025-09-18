#pragma once
#include "DumpStringGenerator.hpp"

namespace Cool {

inline auto get_extra_dump_info() -> std::function<void(DumpStringGenerator&)>&
{
    static auto instance = std::function<void(DumpStringGenerator&)>{};
    return instance;
}

inline void set_extra_dump_info(std::function<void(DumpStringGenerator&)> const& callback)
{
    get_extra_dump_info() = callback;
}

} // namespace Cool