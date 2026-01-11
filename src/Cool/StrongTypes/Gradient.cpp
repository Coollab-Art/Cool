#include "Gradient.h"
#include <imgui.h>
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/ImGuiExtrasStyle.h"

namespace Cool {

auto to_string(Cool::Gradient const&) -> std::string
{
    return "Gradient [...]";
}

auto imgui_widget(std::string_view name, Cool::Gradient& gradient, bool should_use_a_random_color_for_the_new_marks, ImGuiColorEditFlags flags) -> bool
{
    auto modified = gradient.value.widget(
        name.data(),
        {
            .gradient_subdivisions                       = 1,
            .flags                                       = ImGG::Flag::NoResetButton,
            .color_edit_flags                            = flags,
            .should_use_a_random_color_for_the_new_marks = should_use_a_random_color_for_the_new_marks,
            .plus_button_widget                          = []() { return ImGuiExtras::button_with_text_icon(ICOMOON_PLUS); },
            .minus_button_widget                         = []() { return ImGuiExtras::button_with_text_icon(ICOMOON_MINUS); },
            .mark_color                                  = ImGui::ColorConvertFloat4ToU32(ImGuiExtras::GetStyle().gradient_mark),
            .mark_hovered_color                          = ImGui::ColorConvertFloat4ToU32(ImGuiExtras::GetStyle().gradient_mark_hovered),
            .mark_selected_color                         = ImGui::ColorConvertFloat4ToU32(ImGuiExtras::GetStyle().gradient_mark_selected),
            .mark_selected_hovered_color                 = ImGui::ColorConvertFloat4ToU32(ImGuiExtras::GetStyle().gradient_mark_selected_hovered),
        }
    );
    modified |= ImGG::interpolation_mode_widget("##Interpolation Mode", &gradient.value.gradient().interpolation_mode());
    ImGui::SameLine();
    modified |= ImGG::wrap_mode_widget("##Wrap Mode", &gradient.wrap_mode);
    ImGui::SameLine();
    if (ImGui::Button("Equalize"))
    {
        gradient.value.gradient().distribute_marks_evenly();
        modified = true;
    }
    return modified;
}

namespace internal {
auto gradient_marks_array_name(std::string_view name) -> std::string
{
    return fmt::format("{}_", name);
}
} // namespace internal

} // namespace Cool