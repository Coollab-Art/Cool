#pragma once
#include "ImGuiExtrasStyle.h"
#include "imgui-node-editor/imgui_node_editor.h"
#include "imgui.h"

namespace Cool {

inline auto imgui_style_unscaled() -> ImGuiStyle&
{
    static auto instance = ImGuiStyle{};
    return instance;
}

inline auto imgui_extras_style_unscaled() -> ImGuiExtras::Style&
{
    static auto instance = ImGuiExtras::Style{};
    return instance;
}

inline auto imgui_nodes_style_unscaled() -> ax::NodeEditor::Style&
{
    static auto instance = ax::NodeEditor::Style{};
    return instance;
}

} // namespace Cool