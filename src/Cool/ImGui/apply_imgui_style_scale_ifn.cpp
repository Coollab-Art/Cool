#include "apply_imgui_style_scale_ifn.hpp"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/UI Scale/ui_scale.hpp"
#include "imgui-node-editor/imgui_node_editor.h"
#include "imgui.h"
#include "imgui_style_unscaled.hpp"
#include "need_to_apply_imgui_style_scale.hpp"

namespace Cool {

void apply_imgui_style_scale_ifn()
{
    if (!need_to_apply_imgui_style_scale())
        return;
    need_to_apply_imgui_style_scale() = false;

    if (Cool::DebugOptions::log_ui_scale_changes())
        Cool::Log::info("UI Scale", "Applied to style");

    {
        auto const old_colors = ImGui::GetStyle().Colors;
        ImGui::GetStyle()     = imgui_style_unscaled();
        ImGui::GetStyle().ScaleAllSizes(ui_scale());
        ImGui::GetStyle().Colors = old_colors;
    }

    {
        imgui_extras_style_unscaled().copy_all_sizes_to(ImGuiExtras::GetStyle());
        ImGuiExtras::GetStyle().scale_all_sizes(ui_scale());
    }

    {
        auto const old_colors      = ax::NodeEditor::GetStyle().Colors;
        ax::NodeEditor::GetStyle() = imgui_nodes_style_unscaled();
        ax::NodeEditor::GetStyle().ScaleAllSizes(ui_scale());
        ax::NodeEditor::GetStyle().Colors = old_colors;
    }
}

} // namespace Cool