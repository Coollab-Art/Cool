#include "nodes_style_editor.h"
#include "Cool/ImGui/ImGuiExtrasStyle.h"
#include "Cool/ImGui/imgui_style_unscaled.hpp"
#include "Cool/ImGui/need_to_apply_imgui_style_scale.hpp"
#include "imgui-node-editor/imgui_node_editor.h"

namespace Cool {

auto nodes_style_editor() -> bool
{
    bool  b{false};
    auto& style = imgui_nodes_style_unscaled();

    ImGui::Spacing();
    // b |= ImGui::DragFloat4("Node Padding", &style.NodePadding.x, 0.1f, 0.0f, 40.0f);
    b |= ImGuiExtras::GetStyle().imgui_node_title_vertical_alignment();
    b |= ImGui::DragFloat("Node Rounding", &style.NodeRounding, 0.1f, 0.0f, 40.0f);
    b |= ImGui::DragFloat("Node Border Width", &style.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
    b |= ImGui::DragFloat("Hovered Node Border Width", &style.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
    b |= ImGui::DragFloat("Hovered Node Border Offset", &style.HoveredNodeBorderOffset, 0.1f, 0.0f, 15.0f);
    b |= ImGui::DragFloat("Selected Node Border Width", &style.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
    b |= ImGui::DragFloat("Selected Node Border Offset", &style.SelectedNodeBorderOffset, 0.1f, 0.0f, 15.0f);
    // b |= ImGui::DragFloat("Pin Rounding", &style.PinRounding, 0.1f, 0.0f, 40.0f);
    // b |= ImGui::DragFloat("Pin Border Width", &style.PinBorderWidth, 0.1f, 0.0f, 15.0f);
    b |= ImGui::DragFloat("Link Curvature", &style.LinkStrength, 1.0f, 0.0f, 500.0f);
    // ImVec2  SourceDirection;
    // ImVec2  TargetDirection;
    b |= ImGui::DragFloat("Scroll Duration", &style.ScrollDuration, 0.001f, 0.0f, 2.0f);
    // b |= ImGui::DragFloat("Flow Marker Distance", &style.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
    // b |= ImGui::DragFloat("Flow Speed", &style.FlowSpeed, 1.0f, 1.0f, 2000.0f);
    // b |= ImGui::DragFloat("Flow Duration", &style.FlowDuration, 0.001f, 0.0f, 5.0f);
    // b |= ImGui::DragFloat2("Pivot Alignment", (float*)&style.PivotAlignment);
    // b |= ImGui::DragFloat2("Pivot Size", (float*)&style.PivotSize);
    // b |= ImGui::DragFloat2("Pivot Scale", (float*)&style.PivotScale);
    // b |= ImGui::DragFloat("Pin Corners", &style.PinCorners);
    // b |= ImGui::DragFloat("Pin Radius", &style.PinRadius, 0.001f, 0.0f, 5.0f);
    // b |= ImGui::DragFloat("Pin Arrow Size", &style.PinArrowSize, 0.001f, 0.0f, 5.0f);
    // b |= ImGui::DragFloat("Pin Arrow Width", &style.PinArrowWidth, 0.001f, 0.0f, 5.0f);
    b |= ImGui::DragFloat("Group Rounding", &style.GroupRounding, 0.1f, 0.0f, 40.0f);
    b |= ImGui::DragFloat("Group Border Width", &style.GroupBorderWidth, 0.1f, 0.0f, 15.0f);
    // b |= ImGui::DragFloat("Highlight Connected Links", &style.HighlightConnectedLinks);
    // ImGui::DragFloat("Snap Link To Pin Dir", &style.SnapLinkToPinDir);

    if (b)
        need_to_apply_imgui_style_scale() = true;

    return b;
}

} // namespace Cool