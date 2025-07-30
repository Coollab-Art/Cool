#include "Exporter.h"
#include "ExporterU.h"

namespace Cool {

Exporter::Exporter(Exporter&& o) noexcept
    : _gui{std::move(o._gui)}
// We don't move the export process because it is immovable + we typically don't want to move it, we just want to copy the params in the gui
{
}

auto Exporter::operator=(Exporter&& o) noexcept -> Exporter&
{
    _gui = std::move(o._gui);
    // We don't move the export process because it is immovable + we typically don't want to move it, we just want to copy the params in the gui
    return *this;
}

auto Exporter::clock() const -> Clock const&
{
    assert(_video_export_process.has_value() && "Can only be called if `is_exporting()` is true");
    return _video_export_process->clock(); // NOLINT(bugprone-unchecked-optional-access)
}

// auto Exporter::export_image_with_current_settings_using_a_task(Time time, Time delta_time, Polaroid const& polaroid, PathChecks const& path_checks) -> std::filesystem::path // TODO remove
// {
//     auto path = File::find_available_path(_gui.image_export_path(), path_checks);
//     ExporterU::export_image_using_a_task(_gui.export_size(), time, delta_time, polaroid, path);
//     return path;
// }

auto Exporter::export_image_depending_on_params_using_a_task(Time time, Time delta_time, Polaroid const& polaroid, PathChecks const& path_checks, img::ImageExportParams const& image_export_params, std::function<void(Cool::Event)> const& start_callback, std::function<void(Cool::Event)> const& end_callback) -> std::filesystem::path
{
    auto export_path = create_path_depending_on_params(image_export_params);
    if (!image_export_params.export_file_overwrite)
        export_path = File::find_available_path(export_path, path_checks);
    img::Size const export_size = image_export_params.size ? image_export_params.size.value() : _gui.export_size();

    ExporterU::export_image_using_a_task(export_size, time, delta_time, polaroid, export_path, start_callback, end_callback);
    return export_path;
}

auto Exporter::create_path_depending_on_params(img::ImageExportParams const& image_export_params) -> std::filesystem::path
{
    auto const default_path   = _gui.image_export_path();
    auto const export_folder  = image_export_params.folder ? image_export_params.folder.value() : File::without_file_name(default_path);
    auto const file_name      = image_export_params.filename ? image_export_params.filename.value() : File::file_name_without_extension(default_path);
    auto const file_extension = image_export_params.extension ? image_export_params.extension.value() : File::extension(default_path);

    return File::with_extension((export_folder / file_name), file_extension);
}

} // namespace Cool