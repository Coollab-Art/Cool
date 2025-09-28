#include "JsonAutoSerializer.hpp"
#include "Cool/FileWatcher/FileWatcher.h"
#include "Cool/Path/Path.h"

namespace Cool {

JsonAutoSerializer::JsonAutoSerializer(
    std::filesystem::path const&                      file_name,
    bool                                              autosave_when_destroyed,
    std::function<void(nlohmann::json const&)> const& from_json,
    std::function<void(nlohmann::json&)> const&       to_json,
    bool                                              use_shared_user_data,
    WantsToLogWarnings                                wants_to_log_warnings
)
    : _serializer(file_name, from_json, to_json, use_shared_user_data, wants_to_log_warnings)
    , _file_watcher{std::make_unique<FileWatcher>(_serializer.file_path(), FileWatcher_NoCallbacks() /*No callbacks because we want to ignore the initial error if the file doesn't exist*/)}
    , _autosave_when_destroyed{autosave_when_destroyed}
{
    // Load default user data, to get default values for fields that might not be serialized in UserData (e.g. new fields that have been added in a new version of the software that we open for the first time)
    load(Cool::Path::default_user_data() / file_name);
    // Overwrite values from DefaultUserData with the ones set in UserData
    load();
}
JsonAutoSerializer::~JsonAutoSerializer()
{
    if (_autosave_when_destroyed)
        save();
}

void JsonAutoSerializer::save()
{
    _serializer.save();
    _file_watcher->ignore_change_that_we_just_made_to_the_file();
}

void JsonAutoSerializer::load(std::filesystem::path const& path)
{
    _serializer.load(path);
}

void JsonAutoSerializer::load()
{
    _serializer.load();
}

void JsonAutoSerializer::update()
{
    _file_watcher->update(FileWatcher_Callbacks{
        .on_file_changed = [&](std::filesystem::path const& path) { load(path); },
        .on_path_invalid = [&](std::filesystem::path const& path) { Log::internal_warning("JSON", fmt::format("Can't open file \"{}\"", Cool::File::weakly_canonical(path)), {}, _serializer.wants_to_log_warnings()); }
    });
}

} // namespace Cool