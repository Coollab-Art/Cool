from dataclasses import dataclass


@dataclass
class ColorElement:
    name_in_code: str
    name_in_ui: str
    description: str


floating_button_description = "Small buttons that overlap the View"
floating_button_enabled_description = (
    floating_button_description
    + "\\nThis is the color used when the button is used as a toggle and is currently on."
)


def all_color_elements():
    return [
        ColorElement(
            name_in_code="toggle",
            name_in_ui="Toggle",
            description="",
        ),
        ColorElement(
            name_in_code="toggle_hovered",
            name_in_ui="ToggleHovered",
            description="",
        ),
        ColorElement(
            name_in_code="toggle_bg",
            name_in_ui="ToggleBg",
            description="",
        ),
        ColorElement(
            name_in_code="toggle_bg_hovered",
            name_in_ui="ToggleBgHovered",
            description="",
        ),
        ColorElement(
            name_in_code="checkbox_button",
            name_in_ui="CheckboxButton",
            description="",
        ),
        ColorElement(
            name_in_code="checkbox_button_hovered",
            name_in_ui="CheckboxButtonHovered",
            description="",
        ),
        ColorElement(
            name_in_code="checkbox_button_active",
            name_in_ui="CheckboxButtonActive",
            description="",
        ),
        ColorElement(
            name_in_code="floating_button",
            name_in_ui="FloatingButton",
            description=floating_button_description,
        ),
        ColorElement(
            name_in_code="floating_button_hovered",
            name_in_ui="FloatingButtonHovered",
            description=floating_button_description,
        ),
        ColorElement(
            name_in_code="floating_button_active",
            name_in_ui="FloatingButtonActive",
            description=floating_button_description,
        ),
        ColorElement(
            name_in_code="floating_button_enabled",
            name_in_ui="FloatingButtonEnabled",
            description=floating_button_enabled_description,
        ),
        ColorElement(
            name_in_code="floating_button_hovered_enabled",
            name_in_ui="FloatingButtonHoveredEnabled",
            description=floating_button_enabled_description,
        ),
        ColorElement(
            name_in_code="floating_button_active_enabled",
            name_in_ui="FloatingButtonActiveEnabled",
            description=floating_button_enabled_description,
        ),
        ColorElement(
            name_in_code="buttons_separator",
            name_in_ui="ButtonsSeparator",
            description="Separator used when we glue several buttons together without any spacing between them (e.g. the timeline buttons).",
        ),
        ColorElement(
            name_in_code="highlight_items",
            name_in_ui="HighlightItems",
            description='Used for example when hovering some error messages, to highlight the part of the UI that needs to be used in order to fix the error message. Use the "Test Message Console" debug option to send such an error message.',
        ),
        ColorElement(
            name_in_code="gradient_mark",
            name_in_ui="GradientMark",
            description="",
        ),
        ColorElement(
            name_in_code="gradient_mark_hovered",
            name_in_ui="GradientMarkHovered",
            description="",
        ),
        ColorElement(
            name_in_code="gradient_mark_selected",
            name_in_ui="GradientMarkSelected",
            description="",
        ),
        ColorElement(
            name_in_code="gradient_mark_selected_hovered",
            name_in_ui="GradientMarkSelectedHovered",
            description="",
        ),
        ColorElement(
            name_in_code="node_link",
            name_in_ui="NodeLink",
            description="",
        ),
        ColorElement(
            name_in_code="node_link_hovered",
            name_in_ui="NodeLinkHovered",
            description="",
        ),
        ColorElement(
            name_in_code="url_link",
            name_in_ui="UrlLink",
            description="",
        ),
        ColorElement(
            name_in_code="url_link_hovered",
            name_in_ui="UrlLinkHovered",
            description="",
        ),
        ColorElement(
            name_in_code="frame_node_color",
            name_in_ui="Frame node",
            description="",
        ),
    ]


@dataclass
class StyleSetting:
    name_in_code: str
    name_in_ui: str
    # If None, this will default to `name_in_ui`. You might want to change the `name_in_ui` at some point, but keep the `name_in_json` the same so that you don't break serialization.
    name_in_json: str | None
    description: str
    should_scale_with_ui: bool
    default_value: str
    cpp_type: str
    widget_begin: str
    widget_end: str


def all_style_settings():
    return [
        StyleSetting(
            name_in_code="floating_buttons_spacing",
            name_in_ui="Floating buttons spacing",
            name_in_json=None,
            description=floating_button_description,
            should_scale_with_ui=True,
            default_value="4.f",
            cpp_type="float",
            widget_begin="ImGui::SliderFloat",
            widget_end=' 0.f, 20.f, "%.3f"',
        ),
        StyleSetting(
            name_in_code="frame_padding",
            name_in_ui="Frame padding",
            name_in_json=None,
            description="",
            should_scale_with_ui=True,
            default_value="9.f, 4.f",
            cpp_type="ImVec2",
            widget_begin="ImGui::SliderFloat2",
            widget_end=' 0.f, 20.f, "%.3f"',
        ),
        StyleSetting(
            name_in_code="tab_bar_padding",
            name_in_ui="Tab bar padding",
            name_in_json=None,
            description="",
            should_scale_with_ui=True,
            default_value="9.f, 6.f",
            cpp_type="ImVec2",
            widget_begin="ImGui::SliderFloat2",
            widget_end=' 0.f, 20.f, "%.3f"',
        ),
        StyleSetting(
            name_in_code="menu_bar_spacing",
            name_in_ui="Menu bar spacing",
            name_in_json=None,
            description="",
            should_scale_with_ui=True,
            default_value="4.f, 4.f",
            cpp_type="ImVec2",
            widget_begin="ImGui::SliderFloat2",
            widget_end=' 0.f, 20.f, "%.3f"',
        ),
        StyleSetting(
            name_in_code="node_title_vertical_alignment",
            name_in_ui="Node title vertical alignment",
            name_in_json=None,
            description="",
            should_scale_with_ui=False,
            default_value="0.31f",
            cpp_type="float",
            widget_begin="ImGui::SliderFloat",
            widget_end=" 0.f, 1.f",
        ),
    ]


def style_declaration():
    def code(s: StyleSetting):
        return f"{s.cpp_type} {s.name_in_code}{{{s.default_value}}};"

    return "\n".join(map(code, all_style_settings()))


def style_copy_all_sizes_to():
    def code(s: StyleSetting):
        return (
            f"style.{s.name_in_code} = {s.name_in_code};\n"
            if s.should_scale_with_ui
            else ""
        )

    return "".join(map(code, all_style_settings()))


def style_scale_all_sizes():
    def code(s: StyleSetting):
        return (
            f"{s.name_in_code} = ImTrunc({s.name_in_code} * scale);\n"
            if s.should_scale_with_ui
            else ""
        )

    return "".join(map(code, all_style_settings()))


def json(get_or_set: str):
    def code(s: StyleSetting):
        return f'json_{get_or_set}(json, "{s.name_in_json or s.name_in_ui}", style.{s.name_in_code});'

    return "\n".join(map(code, all_style_settings()))


def json_get():
    return json("get")


def json_set():
    return json("set")


def style_imgui_declarations():
    def code(s: StyleSetting):
        return f"""auto imgui_{s.name_in_code}() -> bool;"""

    return "\n".join(map(code, all_style_settings()))


def style_imgui_definitions():
    def code(s: StyleSetting):
        return f'''
auto Style::imgui_{s.name_in_code}() -> bool
{{
    bool const b = {s.widget_begin}("{s.name_in_ui}", (float*)&{s.name_in_code}, {s.widget_end});
    {f"""
    ImGuiExtras::help_marker("{s.description}");
    """ if s.description else ''
    }
    return b;
}}'''

    return "\n".join(map(code, all_style_settings()))


def style_colors():
    def code(element: ColorElement):
        return f'ImVec4 {element.name_in_code}{{0.f, 0.f, 0.f, 1.f}}; {"// " if element.description else ""}{element.description}'

    return "\n".join(map(code, all_color_elements()))


def register_elements():
    def escape_quotes(string: str):
        return string.replace('"', '\\"')

    def register(element: ColorElement):
        return f"""
        config.register_element(ImStyleEd::Element{{
            .name        = "{element.name_in_ui}",
            .get_color   = []() {{ return ImGuiExtras::GetStyle().{element.name_in_code}; }},
            .set_color   = [](ImVec4 const& color) {{ ImGuiExtras::GetStyle().{element.name_in_code} = color; }},
            .description = "{escape_quotes(element.description)}",
        }});
    """

    return "\n".join(map(register, all_color_elements()))


if __name__ == "__main__":
    import os
    from pathlib import Path
    from importlib.machinery import SourceFileLoader

    generate_files = SourceFileLoader(
        "generate_files",
        os.path.join(
            Path(os.path.abspath(__file__)).parent.parent.parent.parent,
            "tooling",
            "generate_files.py",
        ),
    ).load_module()

    generate_files.generate(
        folder="generated_style_extras",
        files=[
            style_colors,
            register_elements,
            style_declaration,
            style_imgui_definitions,
            style_imgui_declarations,
            style_copy_all_sizes_to,
            style_scale_all_sizes,
            json_get,
            json_set,
        ],
    )
