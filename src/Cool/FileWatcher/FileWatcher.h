#pragma once
#include <filesystem>
#include "Cool/File/File.h"

namespace Cool {

struct FileWatcher_Config {
    /// In seconds. The smaller the delay the bigger the performance cost.
    float delay_between_checks = 0.5f;
};

struct FileWatcher_Callbacks {
    /// Called whenever the file changes. Receives the currently watched path as a parameter.
    std::function<void(std::filesystem::path const&)> on_file_changed = [](std::filesystem::path const&) {};
    /// Called whenever the path becomes invalid. Receives the currently watched path as a parameter.
    std::function<void(std::filesystem::path const&)> on_path_invalid = [](std::filesystem::path const& path) {
        Log::internal_warning("File Watcher", fmt::format("Can't open file \"{}\"", Cool::File::weakly_canonical(path)));
    };
};

inline auto FileWatcher_NoCallbacks() -> FileWatcher_Callbacks
{
    return FileWatcher_Callbacks{
        .on_file_changed = [](std::filesystem::path const&) {},
        .on_path_invalid = [](std::filesystem::path const&) {},
    };
}

/// Regularly looks for updates of a file and triggers a callback when the file changes. You must call the update() method on every frame.
class FileWatcher {
public:
    FileWatcher(std::filesystem::path, FileWatcher_Callbacks const&, FileWatcher_Config = {});
    FileWatcher()
        : FileWatcher("", FileWatcher_NoCallbacks())
    {}

    /// Must be called every frame. Checks if the delay is elapsed, if so checks if the file has changed, and if so calls the appropriate callback.
    void update(FileWatcher_Callbacks const&) const;

    /// The path we are currently watching.
    auto path() const -> std::filesystem::path const& { return _path; }

    /// Whether the path we are watching is valid.
    auto path_is_valid() const -> bool;

    /// Sets the path to watch.
    /// The appropriate callback will also be called (either on_file_changed() or on_path_invalid())
    void set_path(std::filesystem::path, FileWatcher_Callbacks const&);

    /// Call this after changing the file that is watched, and we will ignore the change and not call any file_changed callback
    void ignore_change_that_we_just_made_to_the_file();

private:
    void call_appropriate_callback(FileWatcher_Callbacks const& callbacks) const;

private:
    using clock = std::chrono::steady_clock;
    std::filesystem::path                                  _path;
    mutable std::optional<std::filesystem::file_time_type> _time_of_last_change; // nullopt when the file does not exist
    mutable clock::time_point                              _time_of_last_check;
    FileWatcher_Config                                     _config;
};

} // namespace Cool