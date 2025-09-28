# ---HOW TO---

# ------------

from debug_options_generator import DebugOption, generate_debug_options, Kind


def all_debug_options():
    return [
        DebugOption(
            name_in_code="show_framerate_window",
            name_in_ui="Framerate window",
            available_in_release=True,
            window_name="Framerate",
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="show_imgui_demo_window",
            name_in_ui="ImGui Demo window",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="test_all_variable_widgets__window",
            name_in_ui="Test all Variable Widgets",
            kind=Kind.WINDOW,
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="empty_window",
            name_in_ui="Open Empty Window",
            kind=Kind.WINDOW,
            available_in_release=True,
            detailed_description="Useful when you want some blank space in your windows layout.",
        ),
        DebugOption(
            name_in_code="test_message_console__window",
            name_in_ui="Test Message Console",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="test_notifications__window",
            name_in_ui="Test Notifications",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="test_tasks__window",
            name_in_ui="Test Tasks",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="log_when_autosaving",
            name_in_ui="Log when autosaving",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="log_when_rendering_alpha_checkerboard_background",
            name_in_ui="Log when rendering alpha checkerboard background",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="log_when_creating_textures",
            name_in_ui="Log when creating textures",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="log_when_computing_audio_features",
            name_in_ui="Log when computing audio features",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="log_tasks",
            name_in_ui="Log tasks",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="texture_library_debug_view",
            name_in_ui="View Texture Library",
            available_in_release=True,
            window_name="Texture Library",
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="log_opengl_info",
            name_in_ui="Log OpenGL info",
            available_in_release=False,
        ),
        DebugOption(
            name_in_code="log_internal_warnings",
            name_in_ui="Log internal warnings",
            available_in_release=True,
            default_value_in_debug=True,
            default_value_in_release=False,
        ),
        DebugOption(
            name_in_code="log_mouse_position_in_view",
            name_in_ui="Log mouse position in View",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="log_ui_scale_changes",
            name_in_ui="Log UI scale changes",
            available_in_release=True,
        ),
        DebugOption(
            name_in_code="show_command_line_arguments",
            name_in_ui="Show command-line arguments",
            available_in_release=True,
            kind=Kind.BUTTON,
        ),
        DebugOption(
            name_in_code="test_presets__window",
            name_in_ui="Test Presets",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="test_markdown_formatting_window",
            name_in_ui="Test Markdown Formatting",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="emulate_midi_keyboard",
            name_in_ui="Emulate midi keyboard",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="test_tips",
            name_in_ui="Test tips",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="public_exhibition_mode",
            name_in_ui="Public exhibition mode",
            available_in_release=True,
            detailed_description="Currently, simply resets the author info after sharing an image online.",
        ),
        DebugOption(
            name_in_code="style_editor",
            name_in_ui="Style Editor",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="color_themes_editor",
            name_in_ui="Color Themes: Editor",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="color_themes_advanced_config_window",
            name_in_ui="Color Themes: Advanced Config",
            available_in_release=True,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="show_all_icons",
            name_in_ui="Show all icons",
            available_in_release=False,
            kind=Kind.WINDOW,
        ),
        DebugOption(
            name_in_code="imgui_item_picker",
            name_in_ui="ImGui Item Picker",
            available_in_release=False,
            kind=Kind.BUTTON,
            detailed_description='Allows you to click on any ImGui widget and have your IDE break on it, allowing you to find the source code that generated it.\\nShortcut: "CTRL_OR_CMD" + SHIFT + I',
        ),
    ]


if __name__ == "__main__":
    generate_debug_options(
        output_folder="generated",
        namespace="Cool",
        cache_file_name="debug_options_cool",
        debug_options=all_debug_options(),
        wants_to_log_warnings=f"""
        // Makes sure the JSON will skip checking for Cool::DebugOptions to know if it needs to log warnings or not. Otherwise this creates a deadlock in the deserialization of DebugOption's json when it tries to log a warning (when the file is corrupted)
        #if DEBUG
            Cool::WantsToLogWarnings::Always
        #else
            Cool::WantsToLogWarnings::Never
        #endif
        """,
    )
