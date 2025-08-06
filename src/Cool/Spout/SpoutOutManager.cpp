#include "SpoutOutManager.hpp"
#include "Cool/ImGui/ImGuiExtras.h"

namespace Cool {

auto SpoutOutManager::is_active() const -> bool
{
#if defined(COOL_SPOUT)
    return _sender.has_value();
#else
    return false;
#endif
}

void SpoutOutManager::send_texture(Cool::TextureRef texture)
{
#if defined(COOL_SPOUT)
    assert(_sender.has_value());
    _sender->SendTexture(texture.id, GL_TEXTURE_2D, texture.width(), texture.height());
#else
    std::ignore = texture;
#endif
}

void SpoutOutManager::activate_ifn()
{
    if (_need_to_activate)
    {
        create_or_destroy_sender(true);
        _need_to_activate = false;
    }
}

void SpoutOutManager::create_or_destroy_sender(bool is_active)
{
#if defined(COOL_SPOUT)
    if (is_active)
    {
        _sender.emplace();
        _sender->CreateSender(_sender_name.c_str(), _export_size.width(), _export_size.height());
    }
    else
    {
        _sender.reset();
    }
#else
    std::ignore = is_active;
#endif
}

void SpoutOutManager::imgui_window()
{
    _window.show([&](bool /*is_opening*/) {
    // Active
#if defined(COOL_SPOUT)
        bool is_active = _sender.has_value();
        if (Cool::ImGuiExtras::toggle("Active", &is_active))
            create_or_destroy_sender(is_active);
#else
        bool        is_active = false;
        const char* disable_reason =
#if defined(__linux__)
            "Spout and Syphon don't work on Linux. We might look into PipeWire or other solutions at some point";
#elif defined(__APPLE__)
            "Syphon is not yet supported on MacOS";
#else
            "Spout and Syphon are not supported yet";
#endif
        Cool::ImGuiExtras::disabled_if(true, disable_reason, [&]() {
            Cool::ImGuiExtras::toggle("Active", &is_active);
        });
#endif

        // Export size
        _export_size.imgui();

        // Name
        if (ImGui::InputText("Name", &_sender_name))
        {
#if defined(COOL_SPOUT)
            if (_sender.has_value())
            {
                _sender->ReleaseSender();
                _sender->CreateSender(_sender_name.c_str(), _export_size.width(), _export_size.height());
            }
#endif
        }
    });
}

} // namespace Cool