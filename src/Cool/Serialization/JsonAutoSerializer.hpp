#pragma once
#include "JsonSerializer.hpp"

namespace Cool {

class FileWatcher;

/// To add a new serializable type you need to implement to_json() and from_json() for your type (cf. Time.hpp)
class JsonAutoSerializer {
public:
    /// NB: the to_json function should not overwrite the values that are already in the JSON!!!! Otherwise we might remove information that is usefull in another version of the software.
    explicit JsonAutoSerializer(
        std::filesystem::path const&                      file_name,
        bool                                              autosave_when_destroyed,
        std::function<void(nlohmann::json const&)> const& from_json,
        std::function<void(nlohmann::json&)> const&       to_json,
        bool                                              use_shared_user_data,
        WantsToLogWarnings                                wants_to_log_warnings = WantsToLogWarnings::CheckInDebugOption /* HACK: this is for Cool::DebugOption, so it can tell the JSON to skip checking for DebugOptions to know if it needs to log warnings or not. Otherwise this creates a deadlock if the deserialization of DebugOption's json when it tries to log a warning (when the file is corrupted)*/
    );
    ~JsonAutoSerializer();
    JsonAutoSerializer(JsonAutoSerializer const&)                = delete;
    JsonAutoSerializer& operator=(JsonAutoSerializer const&)     = delete;
    JsonAutoSerializer(JsonAutoSerializer&&) noexcept            = delete; // We could implement those one day if we need them
    JsonAutoSerializer& operator=(JsonAutoSerializer&&) noexcept = delete; // We could implement those one day if we need them

    void save();
    void load(std::filesystem::path const& path);
    void load();
    void update();

private:
    JsonSerializer               _serializer;
    std::unique_ptr<FileWatcher> _file_watcher; // We use a pointer because we don't want to include FileWatcher in this header to avoid circular includes
    bool                         _autosave_when_destroyed;
};

} // namespace Cool