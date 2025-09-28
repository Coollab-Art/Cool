#pragma once
#include <filesystem>

namespace Cool {

/// Returns an error message if the check fails, or an empty string if the check is verified
using PathCheck = std::function<std::string(std::filesystem::path const& file_path)>;

auto export_path_existence_check() -> PathCheck;

struct PathChecks {
    std::vector<PathCheck> warnings_checks{export_path_existence_check()};

    auto compute_all_warnings(std::filesystem::path const& file_path) const -> std::vector<std::string>;
    auto is_valid(std::filesystem::path const& file_path) const -> bool;
};

} // namespace Cool