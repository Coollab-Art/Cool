#pragma once
#include "Cool/File/File.h"
#include "nlohmann/json.hpp"

namespace Cool {

struct Event_ExportedImage {
    img::Size             size;
    std::filesystem::path path;

    auto to_json() const -> std::string
    {
        return nlohmann::json{
            {"event", "ImageExportFinished"},
            {"width", size.width()},
            {"height", size.height()},
            {"path", Cool::File::make_absolute(path).string()},
        }
            .dump();
    }
    // TODO(Commands) status success / fail / cancel (if fail, give error message)
};

struct Event_OpenedProject {
    std::filesystem::path path;

    auto to_json() const -> std::string
    {
        return nlohmann::json{
            {"event", "OpenedProject"},
            {"path", Cool::File::make_absolute(path).string()},
        }
            .dump();
    }
};

using Event = std::variant<
    Event_ExportedImage,
    Event_OpenedProject>;

} // namespace Cool