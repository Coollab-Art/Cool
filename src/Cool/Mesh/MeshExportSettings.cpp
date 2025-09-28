#include "MeshExportSettings.hpp"
#include "Cool/File/File.h"
#include "Cool/File/PathChecks.hpp"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Mesh/MeshExportFormat.hpp"
#include "Cool/NfdFileFilter/NfdFileFilter.h"
#include "wafl/wafl.hpp"

namespace Cool {

static auto extensions() -> auto const&
{
    static auto const instance = std::vector<const char*>{".ply"};
    return instance;
}

auto MeshExportSettings::format() const -> MeshExportFormat
{
    const char* extension = wafl::find_best_match(extensions(), File::extension(path).string());
    if (extension == ".ply"sv)
        return MeshExportFormat::ply;
    return MeshExportFormat::ply;
}

void MeshExportSettings::set_file_name_to_an_unused_name()
{
    path = File::find_available_path(path, Cool::PathChecks{});
}

auto MeshExportSettings::imgui() -> bool
{
    bool b = false;
    ImGuiExtras::file_and_folder_saving(path, extensions(), Cool::PathChecks{}, NfdFileFilter::Mesh);
    return b;
}

} // namespace Cool