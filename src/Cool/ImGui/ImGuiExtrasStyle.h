#pragma once
#include <imgui.h>

namespace Cool::ImGuiExtras {

struct Style {
#include "generated_style_extras/style_colors.inl"
#include "generated_style_extras/style_declaration.inl"
#include "generated_style_extras/style_imgui_declarations.inl"

    void copy_all_sizes_to(Style& style);
    void scale_all_sizes(float scale);
};

[[nodiscard]] inline auto GetStyle() -> Style&
{
    static auto instance = Style{};
    return instance;
}

}; // namespace Cool::ImGuiExtras