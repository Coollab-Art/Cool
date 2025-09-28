#include "export_mesh.hpp"
#include <fstream>
#include "Cool/File/File.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

// TODO(Mesh) Return error if export fails
void export_mesh_as_ply(Mesh const& mesh, std::filesystem::path const& path)
{
    auto output_file = std::ofstream{path};

    output_file << "ply\n";
    output_file << "format ascii 1.0\n";
    output_file << "element vertex " << mesh.vertices.size() << '\n';
    output_file << "property float32 x\n";
    output_file << "property float32 y\n";
    output_file << "property float32 z\n";
    output_file << "element face " << mesh.indices.size() << '\n';
    output_file << "property list uint8 int32 vertex_indices\n";
    output_file << "end_header\n";

    for (auto const& vertex : mesh.vertices)
        output_file << vertex.x << " " << vertex.y << " " << vertex.z << '\n';

    for (auto const& face_indices : mesh.indices)
        output_file << "3 " << face_indices.x << " " << face_indices.y << " " << face_indices.z << '\n';
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
        return export_mesh_as_ply(mesh, settings.path);
    }
    }
    assert(false);
    export_mesh_as_ply(mesh, settings.path);
}

}; // namespace Cool