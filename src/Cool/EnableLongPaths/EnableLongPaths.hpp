#pragma once
#if defined(_WIN32)

namespace Cool {

auto has_long_paths_enabled() -> bool;
void enable_long_paths_by_asking_user_permission();

} // namespace Cool

#endif