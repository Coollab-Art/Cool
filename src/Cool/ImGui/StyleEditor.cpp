#include "StyleEditor.h"
#include "Cool/Nodes/nodes_style_editor.h"
#include "Cool/Serialization/Json.hpp"
#include "ImGuiExtras.h"
#include "ImGuiExtrasStyle.h"
#include "imgui-node-editor/imgui_node_editor.h"
#include "imgui.h"
#include "imgui_style_unscaled.hpp"
#include "need_to_apply_imgui_style_scale.hpp"

namespace ax::NodeEditor {

[[maybe_unused]] static void to_json(nlohmann::json& json, ax::NodeEditor::Style const& style)
{
#include "generated_style_nodes/json_set.inl"
}

[[maybe_unused]] static void from_json(nlohmann::json const& json, ax::NodeEditor::Style& style)
{
#include "generated_style_nodes/json_get.inl"
}

} // namespace ax::NodeEditor

[[maybe_unused]] static void to_json(nlohmann::json& json, ImGuiStyle const& style)
{
#include "generated_style/json_set.inl"
}
[[maybe_unused]] static void from_json(nlohmann::json const& json, ImGuiStyle& style)
{
#include "generated_style/json_get.inl"
}

namespace Cool {

namespace ImGuiExtras {
[[maybe_unused]] static void to_json(nlohmann::json& json, ImGuiExtras::Style const& style)
{
#include "generated_style_extras/json_set.inl"
}
[[maybe_unused]] static void from_json(nlohmann::json const& json, ImGuiExtras::Style& style)
{
#include "generated_style_extras/json_get.inl"
}
} // namespace ImGuiExtras

StyleEditor::StyleEditor()
    : _serializer{
          "style.json",
          false /*autosave_when_destroyed*/, // If the user doesn't change the style, we don't want to save it, so that if a new version of the software comes with a new style, if the user hasn't customized the style then we will use the new style from user_data_default
          [](nlohmann::json const& json) {
              json_get(json, "ImGui", imgui_style_unscaled());
              json_get(json, "ImGuiExtras", imgui_extras_style_unscaled());
              json_get(json, "ImNodes", imgui_nodes_style_unscaled());
              need_to_apply_imgui_style_scale() = true;
          },
          [](nlohmann::json& json) {
              json_set(json, "ImGui", imgui_style_unscaled());
              json_set(json, "ImGuiExtras", imgui_extras_style_unscaled());
              json_set(json, "ImNodes", imgui_nodes_style_unscaled());
          },
          true /*use_shared_user_data*/
      }
{}

void StyleEditor::imgui()
{
    auto& style        = imgui_style_unscaled();
    auto& extras_style = imgui_extras_style_unscaled();

    bool b = false;
    ImGuiExtras::separator_text("Rendering");
    b |= ImGui::SliderFloat("Disabled Alpha", &style.DisabledAlpha, 0.f, 1.f);
    ImGuiExtras::help_marker("Additional alpha multiplier for disabled items (multiply over current value of Alpha).");

    ImGuiExtras::separator_text("Main");
    b |= ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.3f");
    b |= extras_style.imgui_frame_padding();
    b |= extras_style.imgui_tab_bar_padding();
    b |= extras_style.imgui_menu_bar_spacing();
    b |= ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.3f");
    b |= ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.3f");
    b |= extras_style.imgui_floating_buttons_spacing();
    b |= ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.3f");
    b |= ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.3f");
    b |= ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.3f");
    b |= ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.3f");

    ImGuiExtras::separator_text("Borders");
    b |= ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.3f");
    b |= ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.3f");
    b |= ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.3f");
    b |= ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.3f");
    b |= ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.3f");
    b |= ImGui::SliderFloat("TabBarBorderSize", &style.TabBarBorderSize, 0.0f, 2.0f, "%.3f");
    b |= ImGui::SliderFloat("DockingSplitterSize", &style.DockingSeparatorSize, 0.0f, 12.0f, "%.3f");

    ImGuiExtras::separator_text("Rounding");
    b |= ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.3f");
    b |= ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.3f");
    b |= ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.3f");
    b |= ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.3f");
    b |= ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.3f");
    b |= ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.3f");
    b |= ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.3f");

    ImGuiExtras::separator_text("Widgets");
    b |= ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
    int window_menu_button_position = style.WindowMenuButtonPosition + 1;
    if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
    {
        style.WindowMenuButtonPosition = window_menu_button_position - 1;
        b                              = true;
    }
    b |= ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
    b |= ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
    ImGuiExtras::help_marker("Alignment applies when a button is larger than its text content.");
    b |= ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
    ImGuiExtras::help_marker("Alignment applies when a selectable is larger than its text content.");
    b |= ImGui::SliderFloat("SeparatorTextBorderSize", &style.SeparatorTextBorderSize, 0.0f, 10.0f, "%.3f");
    b |= ImGui::SliderFloat2("SeparatorTextAlign", (float*)&style.SeparatorTextAlign, 0.0f, 1.0f, "%.2f");
    b |= ImGui::SliderFloat2("SeparatorTextPadding", (float*)&style.SeparatorTextPadding, 0.0f, 40.0f, "%0.f");
    b |= ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.3f");

    ImGui::SeparatorText("Tooltips");
    for (int n = 0; n < 2; n++)
    {
        if (ImGui::TreeNodeEx(n == 0 ? "HoverFlagsForTooltipMouse" : "HoverFlagsForTooltipNav"))
        {
            ImGuiHoveredFlags* p = (n == 0) ? &style.HoverFlagsForTooltipMouse : &style.HoverFlagsForTooltipNav;
            b |= ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayNone", p, ImGuiHoveredFlags_DelayNone);
            b |= ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayShort", p, ImGuiHoveredFlags_DelayShort);
            b |= ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayNormal", p, ImGuiHoveredFlags_DelayNormal);
            b |= ImGui::CheckboxFlags("ImGuiHoveredFlags_Stationary", p, ImGuiHoveredFlags_Stationary);
            b |= ImGui::CheckboxFlags("ImGuiHoveredFlags_NoSharedDelay", p, ImGuiHoveredFlags_NoSharedDelay);
            ImGui::TreePop();
        }
    }
    b |= ImGui::InputFloat("Hover delay short", &style.HoverDelayShort);
    b |= ImGui::InputFloat("Hover delay normal", &style.HoverDelayNormal);
    b |= ImGui::InputFloat("Hover stationary delay", &style.HoverStationaryDelay);

    ImGuiExtras::separator_text("Nodes");
    b |= nodes_style_editor();
    b |= ImGui::SliderFloat("LayoutAlign", &style.LayoutAlign, 0.0f, 1.0f);

    ImGuiExtras::separator_text("Tables");
    b |= ImGui::SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.3f");
    b |= ImGui::SliderAngle("TableAngledHeadersAngle", &style.TableAngledHeadersAngle, -50.0f, +50.0f);

    ImGuiExtras::separator_text("Misc");
    b |= ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.3f");
    ImGuiExtras::help_marker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");

    if (b)
    {
        _serializer.save();
        need_to_apply_imgui_style_scale() = true;
    }
}

} // namespace Cool