#include "TextureSource_Image.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/NfdFileFilter/NfdFileFilter.h"
#include "Cool/TextureSource/TextureLibrary_Image.hpp"
#include "Cool/WebGPU/Texture.h"

namespace Cool {

auto TextureSource_Image::imgui_widget() -> bool
{
    bool b = false;
    b |= ImGuiExtras::file_and_folder_opening("Image Path", &absolute_path, NfdFileFilter::ImageLoad);
    return b;
}

[[nodiscard]] auto TextureSource_Image::get_texture() const -> Texture const*
{
    return texture_library_image().get(absolute_path);
}

auto TextureSource_Image::get_error() const -> std::optional<std::string>
{
    return texture_library_image().error_from(absolute_path);
}

} // namespace Cool