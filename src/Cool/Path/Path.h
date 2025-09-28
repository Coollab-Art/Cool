#pragma once
#include <filesystem>
#include <optional>
#include "Cool/Path/PathsConfig.h"

namespace Cool {

class Path {
public:
    /// Root directory where the program was started from. Use this to describe your ressources by an absolute path, especially if you play at runtime with the directories and your relative paths get messed up
    [[nodiscard]] static auto root() -> std::filesystem::path const&;

    /// Directory where "Cool/res" is located, or where the Cool resources have been copied. This path will always be valid to refer to "Cool/res"
    [[nodiscard]] static auto cool_res() -> std::filesystem::path const&;

    /// Path to the folder where all the user data are stored (imgui.ini, etc.).
    [[nodiscard]] static auto user_data() -> std::filesystem::path const&;

    /// Path to the folder where all the user data are stored (imgui.ini, etc.).
    /// This one is used for files that we want to share between Coollab and the Launcher
    [[nodiscard]] static auto user_data_shared() -> std::filesystem::path const&;

    /// Path to the folder where all the default user data are stored (imgui.ini, etc.). We load things both from this default_user_data, and then from user_data. This allows us to have some default values, that can then be overriden by the user.
    [[nodiscard]] static auto default_user_data() -> std::filesystem::path const&;

    /// Path to the folder where all the initial user data are stored. When a file is missing in the user_data() folder (e.g. the first time you open the app), then it will be copied from initial_user_data folder into user_data.
    [[nodiscard]] static auto initial_user_data() -> std::filesystem::path const&;

    /// Path to the default texture used by TextureInfo.
    [[nodiscard]] static auto default_texture() -> std::filesystem::path const&;

    [[nodiscard]] static auto project_folder() -> std::optional<std::filesystem::path>& { return _project_folder; }

    /// Initializes the paths. This must be called once, at the start of your application.
    template<typename PathsConfigT>
    static void initialize()
    {
        _paths_config = std::make_unique<PathsConfigT>();
    }

private:
    static std::unique_ptr<PathsConfig>         _paths_config;
    static std::optional<std::filesystem::path> _project_folder;
};

} // namespace Cool
