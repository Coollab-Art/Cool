#pragma once
#include "nlohmann/json.hpp"

namespace Cool {

enum class WantsToLogWarnings : uint8_t {
    CheckInDebugOption,
    Always,
    Never,
};

/// To add a new serializable type you need to implement to_json() and from_json() for your type (cf. Time.hpp)
class JsonSerializer {
public:
    /// NB: the to_json function should not overwrite the values that are already in the JSON!!!! Otherwise we might remove information that is usefull in another version of the software.
    explicit JsonSerializer(
        std::filesystem::path const&                      file_name,
        std::function<void(nlohmann::json const&)> const& from_json,
        std::function<void(nlohmann::json&)> const&       to_json,
        bool                                              use_shared_user_data,
        WantsToLogWarnings                                wants_to_log_warnings = WantsToLogWarnings::CheckInDebugOption /* HACK: this is for Cool::DebugOption, so it can tell the JSON to skip checking for DebugOptions to know if it needs to log warnings or not. Otherwise this creates a deadlock if the deserialization of DebugOption's json when it tries to log a warning (when the file is corrupted)*/
    );
    ~JsonSerializer()                                    = default;
    JsonSerializer(JsonSerializer const&)                = delete;
    JsonSerializer& operator=(JsonSerializer const&)     = delete;
    JsonSerializer(JsonSerializer&&) noexcept            = delete; // We could implement those one day if we need them
    JsonSerializer& operator=(JsonSerializer&&) noexcept = delete; // We could implement those one day if we need them

    void save();
    void load(std::filesystem::path const& path);
    void load();

    auto file_path() const -> std::filesystem::path const& { return _file_path; }
    auto wants_to_log_warnings() const -> bool;

private:
    nlohmann::json        _json{}; // We need to store the json, because when we save it we want to keep all the unused fields that we found when we loaded the json. Otherwise we might remove information that is usefull in another version of the software.
    std::filesystem::path _file_path;

    std::function<void(nlohmann::json const&)> _from_json;
    std::function<void(nlohmann::json&)>       _to_json;

    WantsToLogWarnings _wants_to_log_warnings;
};

} // namespace Cool