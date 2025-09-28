#pragma once
#include "nlohmann/json.hpp"

namespace Cool {

enum class VideoExportOverwriteBehaviour {
    AskBeforeCreatingNewFolder,
    AskBeforeOverwritingPreviousFrames,
    AlwaysCreateNewFolder,
    AlwaysOverwritePreviousFrames,
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    VideoExportOverwriteBehaviour,
    {
        {VideoExportOverwriteBehaviour::AskBeforeCreatingNewFolder, "AskBeforeCreatingNewFolder"},
        {VideoExportOverwriteBehaviour::AskBeforeOverwritingPreviousFrames, "AskBeforeOverwritingPreviousFrames"},
        {VideoExportOverwriteBehaviour::AlwaysCreateNewFolder, "AlwaysCreateNewFolder"},
        {VideoExportOverwriteBehaviour::AlwaysOverwritePreviousFrames, "AlwaysOverwritePreviousFrames"},
    }
)

auto imgui_widget(VideoExportOverwriteBehaviour& behaviour) -> bool;

} // namespace Cool