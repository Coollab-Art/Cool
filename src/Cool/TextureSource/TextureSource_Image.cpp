#include "TextureSource_Image.h"
#include <open/open.hpp>
#include "Cool/Gpu/Texture.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/NfdFileFilter/NfdFileFilter.h"
#include "Cool/TextureSource/TextureLibrary_Image.h"

namespace Cool {

auto TextureSource_Image::imgui_widget() -> bool
{
    bool b = false;
    b |= ImGuiExtras::file_and_folder_opening("Image Path", &absolute_path, NfdFileFilter::ImageLoad);
    return b;
}

[[nodiscard]] auto TextureSource_Image::get_texture() const -> Texture const*
{
    return TextureLibrary_Image::instance().get(absolute_path);
}

auto TextureSource_Image::get_error_notification() const -> std::optional<ImGuiNotify::Notification>
{
    auto const err = TextureLibrary_Image::instance().error_from(absolute_path);
    if (!err.has_value())
        return std::nullopt;

    return ImGuiNotify::Notification{
        .type                 = ImGuiNotify::Type::Error,
        .title                = "Image error",
        .content              = fmt::format("\"{}\"\n\n{}", Cool::File::weakly_canonical(absolute_path), *err),
        .custom_imgui_content = [path = absolute_path]() {
            if (ImGui::Button("Try to open file in explorer"))
                Cool::open_focused_in_explorer(path);
        },
        .duration = std::nullopt,
        .closable = false,
    };
}

} // namespace Cool