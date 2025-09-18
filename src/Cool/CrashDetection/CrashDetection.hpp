#pragma once

namespace Cool {

void create_crash_marker(std::filesystem::path const& project_path);
void remove_crash_marker(std::filesystem::path const& project_path);

auto has_crashed(std::filesystem::path const& project_path) -> bool;

} // namespace Cool