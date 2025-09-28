#pragma once

namespace Cool {

/// It is OK to use the Cool::Path methods (e.g. Cool::Path::root()) in these functions.
class PathsConfig {
public:
    PathsConfig()                                      = default;
    PathsConfig(PathsConfig const&)                    = delete;
    auto operator=(PathsConfig const&) -> PathsConfig& = delete;
    PathsConfig(PathsConfig&&)                         = delete;
    auto operator=(PathsConfig&&) -> PathsConfig&      = delete;
    virtual ~PathsConfig()                             = default;

    [[nodiscard]] virtual auto root() const -> std::filesystem::path;
    [[nodiscard]] virtual auto cool_res() const -> std::filesystem::path;
    [[nodiscard]] virtual auto user_data() const -> std::filesystem::path;
    [[nodiscard]] virtual auto user_data_shared() const -> std::filesystem::path;
    [[nodiscard]] virtual auto default_user_data() const -> std::filesystem::path;
    [[nodiscard]] virtual auto initial_user_data() const -> std::filesystem::path;
    [[nodiscard]] virtual auto default_texture() const -> std::filesystem::path;
};

} // namespace Cool