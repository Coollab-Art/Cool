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

auto Exporter::export_image_with_current_settings_using_a_task(Time time, Time delta_time, Polaroid const& polaroid, PathChecks const& path_checks) -> std::filesystem::path // to_remove
{
    auto path = File::find_available_path(_gui.image_export_path(), path_checks);
    ExporterU::export_image_using_a_task(_gui.export_size(), time, delta_time, polaroid, path);
    return path;
}

auto Exporter::export_image_depending_on_params_using_a_task(Time time, Time delta_time, Polaroid const& polaroid, PathChecks const& path_checks, img::ImageParams const& image_params, bool const& override) -> std::filesystem::path
{
    auto export_path = create_path_depending_on_params(image_params);
    if (!override)
        export_path = File::find_available_path(export_path, path_checks);
    img::Size const export_size = image_params.size ? image_params.size.value() : _gui.export_size();

    ExporterU::export_image_using_a_task(export_size, time, delta_time, polaroid, export_path);
    return export_path;
}

auto Exporter::create_path_depending_on_params(img::ImageParams const& image_params) -> std::filesystem::path
{
    auto const default_path  = _gui.image_export_path();
    auto const export_path   = image_params.file_path ? image_params.file_path.value() : File::without_file_name(default_path);
    auto const export_name   = image_params.filename ? image_params.filename.value() : File::file_name_without_extension(default_path);
    auto const export_format = image_params.format ? image_params.format.value() : File::extension(default_path);

    return File::with_extension((export_path / export_name), export_format);
}

} // namespace Cool