#pragma once
#include "Cool/Gpu/Texture.h"
#include "TextureSource_Image.h"
#include "TextureSource_Video.h"
#include "TextureSource_Webcam.h"

namespace Cool {

using TextureSource = std::variant<
    TextureSource_Image,
    TextureSource_Webcam,
    TextureSource_Video>;

auto get_texture(TextureSource const&) -> Texture const&;
auto imgui_widget(TextureSource&) -> bool;
auto get_error_notification(TextureSource const&) -> std::optional<ImGuiNotify::Notification>;

} // namespace Cool