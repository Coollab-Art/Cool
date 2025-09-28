#pragma once
#include "nlohmann/json.hpp"

namespace Cool {

template<typename T>
void json_set(nlohmann::json& json, std::string_view key, T const& val)
{
    to_json(json[key], val); // We don't write `json[key] = val` because we don't want to overwrite values in the json that aren't used anymore, because they might be used by another version of the software.
}

template<typename T>
auto json_get(nlohmann::json const& json, std::string_view key, T& val) -> bool
{
    auto const it = json.find(key);
    if (it == json.end())
        return false;

    try
    {
        from_json(*it, val);
        return true;
    }
    catch (...) // NOLINT(*empty-catch) Ignore the error, it is totally possible that the key is not of the right type
    {}

    return false;
}

} // namespace Cool

inline void to_json(nlohmann::json& json, ImVec2 const& v)
{
    Cool::json_set(json, "x", v.x);
    Cool::json_set(json, "y", v.y);
}
inline void from_json(nlohmann::json const& json, ImVec2& v)
{
    Cool::json_get(json, "x", v.x);
    Cool::json_get(json, "y", v.y);
}

inline void to_json(nlohmann::json& json, ImVec4 const& v)
{
    Cool::json_set(json, "x", v.x);
    Cool::json_set(json, "y", v.y);
    Cool::json_set(json, "z", v.z);
    Cool::json_set(json, "w", v.w);
}
inline void from_json(nlohmann::json const& json, ImVec4& v)
{
    Cool::json_get(json, "x", v.x);
    Cool::json_get(json, "y", v.y);
    Cool::json_get(json, "z", v.z);
    Cool::json_get(json, "w", v.w);
}