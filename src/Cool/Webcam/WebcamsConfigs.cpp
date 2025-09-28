#include "WebcamsConfigs.hpp"
#include "wcam/wcam.hpp"

namespace Cool {

auto WebcamsConfigs::instance() -> WebcamsConfigs&
{
    static auto inst = WebcamsConfigs{}; // NOLINT(*avoid-non-const-global-variables)
    return inst;
}

void WebcamsConfigs::open_imgui_window()
{
    _window.open();
}

void WebcamsConfigs::imgui_window()
{
    _window.show([&](bool /*is_opening*/) {
        auto const webcam_infos = wcam::all_webcams_info();
        for (auto const& info : webcam_infos)
        {
            ImGui::PushID(info.id.as_string().c_str());
            ImGui::SeparatorText(info.name.c_str());
            auto const selected_resolution = wcam::get_selected_resolution(info.id);
            if (ImGui::BeginCombo("Resolution", wcam::to_string(selected_resolution).c_str()))
            {
                for (auto const& resolution : info.resolutions)
                {
                    bool const is_selected = resolution == selected_resolution;
                    if (ImGui::Selectable(wcam::to_string(resolution).c_str(), is_selected))
                        wcam::set_selected_resolution(info.id, resolution);

                    if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
            ImGui::NewLine();
        }
    });
}

} // namespace Cool