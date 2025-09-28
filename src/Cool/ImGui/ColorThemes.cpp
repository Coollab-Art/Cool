#include "ColorThemes.h"
#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Path/Path.h"
#include "ImGuiExtrasStyle.h"

namespace Cool {

static void register_imgui_extras_elements(ImStyleEd::Config& config);

ColorThemes::ColorThemes()
    : _editor{
          ImStyleEd::SerializationPaths{
              .current_theme_file    = Cool::Path::user_data_shared() / "current_color_theme.json",
              .themes_folder         = Cool::Path::user_data_shared() / "Color Themes",
              .themes_default_folder = Cool::Path::default_user_data() / "Color Themes",
              .config_file           = Cool::Path::default_user_data() / "color_config.json",
          },
          ImStyleEd::CurrentTheme{"Dark"}, // Start with Dark theme by default (using "Use OS Theme" means most people will have the Light theme, which is not quite as good as the Dark one)
          [](ImStyleEd::Config& config) {
              ImStyleEd::register_all_imgui_color_elements(config);
              register_imgui_extras_elements(config);
          }
      }
{
}

void ColorThemes::update()
{
    _editor.update();
}

void ColorThemes::imgui_theme_picker()
{
    _editor.imgui_theme_selector();
}

void ColorThemes::imgui_basic_theme_editor()
{
    _editor.imgui_themes_editor();
}

void ColorThemes::imgui_advanced_config()
{
    _editor.imgui_config_editor();
}

static void register_imgui_extras_elements(ImStyleEd::Config& config)
{
#include "generated_style_extras/register_elements.inl"
#include "generated_style_nodes/register_all_imnodes_color_elements.inl"
}

} // namespace Cool
