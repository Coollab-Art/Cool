#include "CrashDetection.hpp"
#include "Cool/File/File.h"

namespace Cool {

static auto crash_marker_path(std::filesystem::path const& project_path) -> std::filesystem::path
{
    auto res = project_path;
    res += ".crash_marker";
    return res;
}

void create_crash_marker(std::filesystem::path const& project_path)
{
    Cool::File::create_file_if_it_doesnt_exist(crash_marker_path(project_path));
}

void remove_crash_marker(std::filesystem::path const& project_path)
{
    Cool::File::remove_file(crash_marker_path(project_path));
}

auto has_crashed(std::filesystem::path const& project_path) -> bool
{
    return Cool::File::exists(crash_marker_path(project_path));
}

} // namespace Cool