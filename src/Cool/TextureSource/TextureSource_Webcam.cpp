#include "TextureSource_Webcam.h"
#include <wcam/wcam.hpp>
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Webcam/WebcamsConfigs.hpp"
#include "TextureLibrary_Webcam.hpp"

namespace Cool {

void TextureSource_Webcam::set_device_id(wcam::DeviceId const& id) const
{
    _device_id = id;
}

auto TextureSource_Webcam::imgui_widget() -> bool
{
    bool b = false;

    auto const webcam_infos = wcam::all_webcams_info();
    if (_device_id == wcam::DeviceId{} && !webcam_infos.empty()) // When creating a new webcam we want it to use the default webcam, but we might not know what the available webcams are at that point (if the wcam library has just been initialized). So we defer that to here
        set_device_id(webcam_infos[0].id);

    auto const it   = std::find_if(webcam_infos.begin(), webcam_infos.end(), [&](wcam::Info const& info) {
        return info.id == _device_id;
    });
    auto const name = it != webcam_infos.end() ? it->name : ""s;

    if (ImGuiExtras::button_with_text_icon(ICOMOON_COG))
        WebcamsConfigs::instance().open_imgui_window();
    ImGui::SetItemTooltip("%s", "Open webcam config to choose its resolution. This can help if the refresh rate of your webcam is too slow.");

    if (ImGui::BeginCombo("Webcam", name.c_str()))
    {
        for (auto const& info : webcam_infos)
        {
            bool const is_selected = info.id == _device_id;
            if (ImGui::Selectable(info.name.c_str(), is_selected))
            {
                set_device_id(info.id);
                b = true;
            }
            if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return b;
}

auto TextureSource_Webcam::get_texture() const -> Texture const*
{
    if (_device_id == wcam::DeviceId{}) // When creating a new webcam we want it to use the default webcam, but we might not know what the available webcams are at that point (if the wcam library has just been initialized). So we defer that to here
    {
        auto const webcam_infos = wcam::all_webcams_info();
        if (!webcam_infos.empty())
            set_device_id(webcam_infos[0].id);
    }
    return TextureLibrary_Webcam::instance().get_texture(_device_id);
}

auto TextureSource_Webcam::get_error() const -> std::optional<std::string>
{
    return TextureLibrary_Webcam::instance().get_error(_device_id);
}

} // namespace Cool