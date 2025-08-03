#pragma once
#include "Cool/Exporter/ExportSize.h"
#include "Cool/Gpu/OpenGL/TextureRef.hpp"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/ImGui/icon_fmt.h"
#if defined(COOL_SPOUT)
#include "SpoutGL/SpoutSender.h"
#endif

namespace Cool {

class SpoutOutManager {
public:
    auto is_active() const -> bool;
    auto texture_size() const -> img::Size { return _export_size; }
    void send_texture(Cool::TextureRef);

    void imgui_window();
    void open_imgui_window() { _window.open(); }

    void activate_ifn();
    void set_shared_aspect_ratio(Cool::SharedAspectRatio& shared_aspect_ratio) { _export_size.set_shared_aspect_ratio(shared_aspect_ratio); }

private:
    void create_or_destroy_sender(bool is_active);

private:
#if defined(COOL_SPOUT)
    std::optional<SpoutSender> _sender{};
#endif

    Cool::ExportSize _export_size{};
    std::string      _sender_name{COOL_APP_NAME};

    Cool::ImGuiWindow _window{Cool::icon_fmt("Spout/Syphon OUT", ICOMOON_IMAGE), Cool::ImGuiWindowConfig{.is_modal = false, .start_open = false}};

    bool _need_to_activate{false}; // HACK because when we deserialize a project, the OpenGL context is not created yet so we can't create the sender immediately

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void save(Archive& archive) const
    {
#if defined(COOL_SPOUT)
        bool is_active{_sender.has_value()};
#else

        bool is_active{false};
#endif
        archive(
            ser20::make_nvp("Is Active", is_active),
            ser20::make_nvp("Size", _export_size),
            ser20::make_nvp("Name", _sender_name),
            ser20::make_nvp("Window", _window)
        );
    }
    template<class Archive>
    void load(Archive& archive)
    {
        archive(
            _need_to_activate,
            _export_size,
            _sender_name,
            _window
        );
    }
};

} // namespace Cool
