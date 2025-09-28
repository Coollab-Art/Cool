#pragma once
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/ImGui/icon_fmt.h"
#include "wcam/wcam.hpp"

namespace Cool {

class WebcamsConfigs {
public:
    [[nodiscard]] static auto instance() -> WebcamsConfigs&;

    void open_imgui_window();
    void imgui_window();

private:
    WebcamsConfigs() = default; // This is a singleton. Get the global instance with `instance()` instead.
    static auto gen_instance() -> WebcamsConfigs&;

private:
    ImGuiWindow _window{icon_fmt("Webcams", ICOMOON_VIDEO_CAMERA)};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Window", _window),
            ser20::make_nvp("Resolutions", wcam::get_resolutions_map())
        );
    }
};

} // namespace Cool