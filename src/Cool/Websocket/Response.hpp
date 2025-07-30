#pragma once
#include "Event.hpp"

namespace Cool {

struct Response {
    int   id{};
    Event event;

    auto to_json() const -> std::string
    {
        auto json           = std::visit([](const auto& event) { return nlohmann::json::parse(event.to_json()); }, event);
        json["response_id"] = id;
        return json.dump();
    }
};

} // namespace Cool