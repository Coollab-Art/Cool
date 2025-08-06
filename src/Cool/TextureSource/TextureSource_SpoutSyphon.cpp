#include "TextureSource_SpoutSyphon.hpp"
#include "Cool/ImGui/ImGuiExtras_dropdown.hpp"
#include "TextureLibrary_Spout.hpp"

namespace Cool {

// If we have no sender name (e.g. there was none connected when we created the texture source)
// then we want to connect automatically to the first one that becomes available
void TextureSource_SpoutSyphon::init_sender_name_ifn() const
{
#if defined(COOL_SPOUT)
    if (_sender_name != "")
        return;

    auto const sender_names = texture_library_spout().get_sender_names();
    if (sender_names.empty())
        return;

    _sender_name = sender_names[0];
#endif
}

auto TextureSource_SpoutSyphon::imgui_widget() -> bool
{
    init_sender_name_ifn();
#if defined(COOL_SPOUT)
    return ImGuiExtras::dropdown("Spout Sender", &_sender_name, texture_library_spout().get_sender_names());
#else
    return false;
#endif
}

auto TextureSource_SpoutSyphon::get_texture() const -> Texture const*
{
    init_sender_name_ifn();
#if defined(COOL_SPOUT)
    return texture_library_spout().get_texture(_sender_name);
#else
    return nullptr;
#endif
}

auto TextureSource_SpoutSyphon::get_error_notification() const -> std::optional<ImGuiNotify::Notification>
{
    auto error_message = ""s;
#if defined(COOL_SPOUT)

    init_sender_name_ifn();
    if (_sender_name.empty())
        error_message = "No Spout sender is connected";
    else
    {
        auto const err = texture_library_spout().get_error(_sender_name);
        if (!err.has_value())
            return std::nullopt;
        error_message = fmt::format("\"{}\"\n\n{}", _sender_name, *err);
    }

#else
#if defined(__linux__)
    error_message = "Spout and Syphon don't work on Linux. We might look into PipeWire or other solutions at some point";
#elif defined(__APPLE__)
    error_message = "Syphon is not yet supported on MacOS";
#else
    error_message = "Spout and Syphon are not supported yet";
#endif
#endif
    return ImGuiNotify::Notification{
        .type     = ImGuiNotify::Type::Error,
        .title    = "Spout/Syphon error",
        .content  = error_message,
        .duration = std::nullopt,
        .closable = false,
    };
}

} // namespace Cool