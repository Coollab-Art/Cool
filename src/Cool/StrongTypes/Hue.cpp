#include "Hue.h"
#include "imgui.h"

namespace Cool {

auto Hue::imgui_widget(std::string_view name) -> bool
{
    bool b = false;
    ImGui::PushID(name.data());

    b |= ImGuiExtras::hue_wheel(name.data(), &value);

    ImGui::SameLine();

    int as_int = static_cast<int>(value * 360.f);
    if (ImGui::SliderInt("", &as_int, 0, 360))
    {
        b     = true;
        value = static_cast<float>(as_int) / 360.f;
    }

    ImGui::PopID();
    return b;
}

} // namespace Cool