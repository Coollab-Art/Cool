#include "ImGuiExtrasStyle.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace Cool::ImGuiExtras {

#include "generated_style_extras/style_imgui_definitions.inl"

void Style::copy_all_sizes_to(Style& style)
{
#include "generated_style_extras/style_copy_all_sizes_to.inl"
}

void Style::scale_all_sizes(float scale)
{
#include "generated_style_extras/style_scale_all_sizes.inl"
}

} // namespace Cool::ImGuiExtras