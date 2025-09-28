#include "FileWatcher.h"
#include <Cool/File/File.h>
#include <filesystem>

namespace Cool {

static auto time_of_last_change(std::filesystem::path const& path) -> std::optional<std::filesystem::file_time_type>
{
    try
    {
        if (!std::filesystem::is_regular_file(path)) // The file watcher doesn't accept directories nor special files, only regular files. Returning nullopt will make us consider that the path is invalid.
            return std::nullopt;
        return std::filesystem::last_write_time(path);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

FileWatcher::FileWatcher(std::filesystem::path path, FileWatcher_Callbacks const& callbacks, FileWatcher_Config config)
    : _config{config}
{
    set_path(std::move(path), callbacks);
}

void FileWatcher::set_path(std::filesystem::path path, FileWatcher_Callbacks const& callbacks)
{
    _path                = std::move(path);
    _time_of_last_change = time_of_last_change(_path);
    _time_of_last_check  = clock::now();
    call_appropriate_callback(callbacks);
}

void FileWatcher::ignore_change_that_we_just_made_to_the_file()
{
    _time_of_last_change = time_of_last_change(_path);
}

void FileWatcher::update(FileWatcher_Callbacks const& callbacks) const
{
    { // Wait for the delay between checks
        auto const now          = clock::now();
        auto const elapsed_time = std::chrono::duration<float>{now - _time_of_last_check};
        if (elapsed_time.count() < _config.delay_between_checks)
            return;
        _time_of_last_check = now;
    }

    { // Check if the file was updated since last check
        auto const last_change = time_of_last_change(_path);
        if (last_change == _time_of_last_change)
            return;
        _time_of_last_change = last_change;
    }

    call_appropriate_callback(callbacks);
}

void FileWatcher::call_appropriate_callback(FileWatcher_Callbacks const& callbacks) const
{
    if (path_is_valid())
        callbacks.on_file_changed(_path);
    else
        callbacks.on_path_invalid(_path);
}

auto FileWatcher::path_is_valid() const -> bool
{
    return _time_of_last_change != std::nullopt;
}

} // namespace Cool