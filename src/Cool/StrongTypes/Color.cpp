#include "Color.h"
#include "Cool/ColorSpaces/conversions.h"
#include "Cool/ImGui/ColorThemes.h"
#include "ImGui_StyleEditor/src/Color.hpp"

namespace Cool {

Color::Color(ImStyleEd::Color const& color)
    : _srgb{color.r, color.g, color.b}
{}

auto Color::from_srgb(glm::vec3 const& srgb) -> Color
{
    Color res{};
    res._srgb = srgb;
    return res;
}

#include "Cool/ColorSpaces/generated/define_color_getters.inl"

auto Color::as_ImColor() const -> ImColor
{
    auto const srgb = as_sRGB();
    return ImColor{srgb.r, srgb.g, srgb.b, 1.f};
}

auto to_string(Color const& color) -> std::string
{
    return fmt::format("{} (sRGB)", glm::to_string(color.as_sRGB()));
}

auto imgui_widget(std::string_view name, Color& color, ImGuiColorEditFlags flags) -> bool
{
    glm::vec3 srgb = color.as_sRGB();
    if (ImGui::ColorEdit3(
            name.data(),
            glm::value_ptr(srgb),
            flags
        ))
    {
        color = Color::from_srgb(srgb);
        return true;
    }
    return false;
}

auto Color::brighter() const -> Color
{
    return Color::from_srgb(as_sRGB() * 1.3f);
}

auto get_text_color(Color const& background_color) -> Color
{
    bool const color_is_bright = background_color.as_Oklab().x > 0.75f;
    auto const color           = color_themes()->editor().get_color_from_theme_or_default(color_is_bright ? "Light" : "Dark", "Text");
    return Color::from_srgb({color.r, color.g, color.b});
}

} // namespace Cool