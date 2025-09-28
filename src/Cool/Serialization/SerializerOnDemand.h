#pragma once
#include <concepts>
#include <utility>
#include "Cool/Serialization/Serialization.h"

namespace Cool {

class SerializerOnDemand {
public:
    SerializerOnDemand(
        std::filesystem::path path,
        std::string_view      key_name_in_json
    )
        : _path{std::move(path)}
        , _key_name_in_json{key_name_in_json}
    {}

    template<typename T, typename InputArchive>
    auto load(T& object) const -> tl::expected<void, ErrorMessage>
    {
        return Serialization::load<T, InputArchive>(object, _path);
    }

    template<typename T, typename OutputArchive>
    auto save(T const& object) const -> bool
    {
        return Serialization::save<T, OutputArchive>(object, _path, _key_name_in_json);
    }

    [[nodiscard]] auto path() const -> auto const& { return _path; }

private:
    std::filesystem::path _path;
    std::string           _key_name_in_json;
};

} // namespace Cool
