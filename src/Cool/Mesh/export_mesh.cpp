#include "export_mesh.hpp"
#include <fstream>
#include "Cool/File/File.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

static auto export_mesh_as_ply(Mesh const& mesh, std::filesystem::path const& path) -> bool
{
    return File::transactional_save(path, [&](std::ofstream& ofs) {
        ofs << "ply\n";
        ofs << "format ascii 1.0\n";
        ofs << "element vertex " << mesh.vertices.size() << '\n';
        ofs << "property float32 x\n";
        ofs << "property float32 y\n";
        ofs << "property float32 z\n";
        ofs << "element face " << mesh.indices.size() << '\n';
        ofs << "property list uint8 int32 vertex_indices\n";
        ofs << "end_header\n";

        for (auto const& vertex : mesh.vertices)
            ofs << vertex.x << " " << vertex.y << " " << vertex.z << '\n';

        for (auto const& face_indices : mesh.indices)
            ofs << "3 " << face_indices.x << " " << face_indices.y << " " << face_indices.z << '\n';
    });
}

// TODO(Mesh) Return error if export fails
void export_mesh(Mesh const& mesh, MeshExportSettings const& settings)
{
    if (!File::create_folders_for_file_if_they_dont_exist(settings.path))
    {
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Warning,
            .title   = "3D Model Export",
            .content = fmt::format("Failed to create the folder \"{}\"", Cool::File::weakly_canonical(settings.path)),
        });
        return;
    }
    switch (settings.format())
    {
    case MeshExportFormat::ply:
    {
        export_mesh_as_ply(mesh, settings.path); // TODO(Mesh) Return error if export fails
    }
    }
    assert(false);
    export_mesh_as_ply(mesh, settings.path);
}

}; // namespace Cool