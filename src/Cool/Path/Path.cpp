#include "Path.h"
#include <utility>

namespace Cool {

std::unique_ptr<PathsConfig>         Path::_paths_config{nullptr};
std::optional<std::filesystem::path> Path::_project_folder{};

auto Path::root() -> std::filesystem::path const&
{
#if DEBUG // On some compilers asserts are compiled even when not in DEBUG
    assert(_paths_config != nullptr && "Path::initialize() has not been called yet!");
#endif
    static auto const path = _paths_config->root();
    return path;
}

auto Path::cool_res() -> std::filesystem::path const&
{
#if DEBUG // On some compilers asserts are compiled even when not in DEBUG
    assert(_paths_config != nullptr && "Path::initialize() has not been called yet!");
#endif
    static auto const path = _paths_config->cool_res();
    return path;
}

auto Path::user_data() -> std::filesystem::path const&
{
#if DEBUG // On some compilers asserts are compiled even when not in DEBUG
    assert(_paths_config != nullptr && "Path::initialize() has not been called yet!");
#endif
    static auto const path = _paths_config->user_data();
    return path;
}

auto Path::user_data_shared() -> std::filesystem::path const&
{
#if DEBUG // On some compilers asserts are compiled even when not in DEBUG
    assert(_paths_config != nullptr && "Path::initialize() has not been called yet!");
#endif
    static auto const path = _paths_config->user_data_shared();
    return path;
}

auto Path::default_user_data() -> std::filesystem::path const&
{
#if DEBUG // On some compilers asserts are compiled even when not in DEBUG
    assert(_paths_config != nullptr && "Path::initialize() has not been called yet!");
#endif
    static auto const path = _paths_config->default_user_data();
    return path;
}

auto Path::initial_user_data() -> std::filesystem::path const&
{
#if DEBUG // On some compilers asserts are compiled even when not in DEBUG
    assert(_paths_config != nullptr && "Path::initialize() has not been called yet!");
#endif
    static auto const path = _paths_config->initial_user_data();
    return path;
}

auto Path::default_texture() -> std::filesystem::path const&
{
#if DEBUG // On some compilers asserts are compiled even when not in DEBUG
    assert(_paths_config != nullptr && "Path::initialize() has not been called yet!");
#endif
    static auto const path = _paths_config->default_texture();
    return path;
}

} // namespace Cool
