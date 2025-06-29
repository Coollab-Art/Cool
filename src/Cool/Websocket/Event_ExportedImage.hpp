#pragma once

namespace Cool {

struct Event_ExportedImage {
    img::Size             size;
    std::filesystem::path path;
    // TODO(Commands) status success / fail / cancel (if fail, give error message)
};

} // namespace Cool