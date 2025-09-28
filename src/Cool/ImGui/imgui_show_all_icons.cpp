#include "imgui_show_all_icons.hpp"
#include <imgui.h>
#include "IcoMoonCodepoints.h"
#include "ImGuiExtras.h"

namespace Cool {

void imgui_show_all_icons()
{
    ImGuiExtras::fill_layout("##icons", 50.f, [](std::function<void()> const& next_item) {
#include "generated_icons/imgui_show_all_icons.inl"
    });
}

} // namespace Cool