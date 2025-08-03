#include "TextureSource_SpoutSyphon.hpp"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras_dropdown.hpp"
#include "TextureLibrary_Spout.hpp"
// #include "TextureLibrary_Spout.hpp"

namespace Cool {

void TextureSource_SpoutSyphon::init_sender_name_ifn() const
{
    // If we have no sender name (e.g. there was none connected when we created the texture source)
    // Then we want to connect automatically to the first one that becomes available

    if (_sender_name != "")
        return;

    auto const sender_names = texture_library_spout().get_sender_names();
    if (sender_names.empty()) // When creating a new webcam we want it to use the default webcam, but we might not know what the available webcams are at that point (if the wcam library has just been initialized). So we defer that to here
        return;

    _sender_name = sender_names[0];
}

auto TextureSource_SpoutSyphon::imgui_widget() -> bool
{
    init_sender_name_ifn();
    return ImGuiExtras::dropdown("Spout Sender", &_sender_name, texture_library_spout().get_sender_names());
}

auto TextureSource_SpoutSyphon::get_texture() const -> Texture const*
{
    init_sender_name_ifn();
    // TODO(Spout) error message if on linux or mac
    return texture_library_spout().get_texture(_sender_name);
}

auto TextureSource_SpoutSyphon::get_error_notification() const -> std::optional<ImGuiNotify::Notification>
{
    auto const err = texture_library_spout().get_error(_sender_name);
    if (!err.has_value())
        return std::nullopt;

    return ImGuiNotify::Notification{
        .type     = ImGuiNotify::Type::Error,
        .title    = "Spout error", // TODO(Spout) or "Syphon error"
        .content  = fmt::format("\"{}\"\n\n{}", _sender_name, *err),
        .duration = std::nullopt,
        .closable = false,
    };
}

} // namespace Cool