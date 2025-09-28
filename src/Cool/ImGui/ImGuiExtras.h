#pragma once
#include <filesystem>
#include "Cool/File/File.h"
#include "Cool/File/PathChecks.hpp"
#include "Cool/StrongTypes/Color.h"

namespace Cool::ImGuiExtras {

/// A small info icon that displays some text when hovered.
void help_marker(const char* text);

/// A building block for help_marker(). In most cases you should just use help_marker(). This is only needed when you need to customize how help_marker() is rendered.
void help_marker_icon();
/// A building block for help_marker(). In most cases you should just use help_marker(). This is only needed when you need to customize how help_marker() is rendered.
void help_marker_tooltip_content(const char* text);

/// A widget to pick an angle (in radians), displayed as a direction on a wheel
/// When SHIFT is pressed (or `always_snap` is set to true) the angle will snap to some precise angles. `number_of_snaps` controls the number of such angles, and `snaps_offset` will offset these.
/// `number_of_snaps` must be >= 1. `snaps_offset` is expressed in radians.
auto angle_wheel(const char* label, float* value_p, int number_of_snaps = 24, float snaps_offset = 0.f, bool always_snap = false) -> bool;

auto angle_slider(const char* label, float* value_p) -> bool;

bool direction_3d(const char* label, float* value_p1, float* value_p2);

/**
 * @brief A greyed out button that you can't click on.
 *
 * @param label The text inside the button
 * @param reason_for_disabling An explanation that will be shown on hover
 */
void button_disabled(const char* label, const char* reason_for_disabling = "Currently disabled");

/// A button colored with the given `hue` (a number between 0 and 1).
auto colored_button(const char* label, float hue, const ImVec2& size = ImVec2(0, 0)) -> bool;
auto colored_button(const char* label, Color color, const ImVec2& size = ImVec2(0, 0)) -> bool;

/**
 * @brief A button that uses an image instead of text
 *
 * @param tex_id OpenGL texture ID of the image
 * @param tint_color You can tint your image
 * @param background_color Color that will appear in places where your image is transparent
 * @param button_width
 * @param button_height
 * @param frame_padding
 * @return true iff the button was just pressed
 */
bool button_with_icon(ImTextureID tex_id, const ImVec4& tint_color = ImVec4(1, 1, 1, 1), const ImVec4& background_color = ImVec4(0, 0, 0, 1), float button_width = 18.f, float button_height = 18.f, int frame_padding = 1);

/**
 * @brief A button that uses an image instead of text, but greyed out and you wan't click on it.
 *
 * @param tex_id OpenGL texture ID of the image
 * @param reason_for_disabling An explanation that will be shown on hover
 * @param button_width
 * @param button_height
 * @param frame_padding
 */
void button_with_icon_disabled(ImTextureID tex_id, const char* reason_for_disabling = "Currently disabled", float button_width = 18.f, float button_height = 18.f, std::optional<float> frame_padding = 1.f);

///
auto button_with_text_icon(const char* icon, ImDrawFlags = 0) -> bool;

/// A checkbox with the appearance of a button with a given icon.
auto checkbox_button(const char* icon, bool* v) -> bool;

/// Draws a clickable red cross.
auto close_button() -> bool;

struct image_framed_options {
    std::optional<float> frame_thickness       = std::nullopt;
    ImVec4               frame_color           = ImVec4(0, 0, 0, 0);
    ImVec4               background_color      = ImVec4(0, 0, 0, 1);
    ImVec4               tint_color            = ImVec4(1, 1, 1, 1);
    bool                 flip_y                = false;
    ImTextureID          background_texture_id = nullptr;
};
/**
 * @brief Displays an image with a frame around it
 *
 * @param tex_id OpenGL texture ID of the image
 * @param size Size the image will be displayed at
 * @param frame_thickness Thickness of the frame. Leave as std::nullopt if you don't want a frame
 * @param frame_color Color of the frame
 * @param background_color Color that will appear in places where your image is transparent
 * @param tint_color You can tint your image
 */
void image_framed(ImTextureID tex_id, const ImVec2& size, image_framed_options const& = {});

/**
 * @brief Like ImGui::InputInt, but for an unsigned int
 *
 * @param label
 * @param value_p
 * @return
 */
bool input_uint(const char* label, unsigned int* value_p);

bool slider_uint32(const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, const char* format = nullptr, ImGuiSliderFlags flags = 0);

/**
 * @brief Writes some red text
 *
 * @param text
 */
void warning_text(const char* text);

/**
 * @brief Opens a context menu when you click on the button. Usage is :
 * if (ImGui::begin_popup_context_menu_from_button("Text")) {
 * 	// some ImGui widgets;
 * 	ImGui::EndPopup();
 * }
 *
 * @param label The text displayed inside the button
 * @param popup_flags The ImGui flags of the context menu
 * @return true iff the popup is open
 */
bool begin_popup_context_menu_from_button(const char* label, ImGuiPopupFlags popup_flags = ImGuiPopupFlags_None);

/**
 * @brief Can be useful for example to open a context menu on a dropdown, which would fail without this wrapper.
 *
 * @param str_id A label that won't be displayed but is used as an ID
 */
void invisible_wrapper_around_previous_line(const char* str_id);

/// Adds a button that opens a folder dialog.
/// Returns true iff out_path was modified.
auto folder_dialog_button(
    std::filesystem::path* out_path,
    File::folder_dialog_args const& = {}
) -> bool;

/// Adds a button that opens a file opening dialog.
/// Returns true iff out_path was modified.
auto file_opening_dialog_button(
    std::filesystem::path* out_path,
    File::file_dialog_args const& = {}
) -> bool;

/// Adds a button that opens a file saving dialog.
/// Returns true iff out_path was modified.
auto file_saving_dialog_button(
    std::filesystem::path* out_path,
    File::file_dialog_args const& = {}
) -> bool;

/// UI for a folder path. Creates a text input and a button to open a folder explorer.
auto folder(
    const char*            label,
    std::filesystem::path* folder_path,
    bool                   show_dialog_button = true
) -> bool;

/// UI for a file path. Creates a text input and a button to open a file explorer.
/// `file_filters` is a set of filters for the file types that should be selectable. Something like { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } }. You can find predefined filters in <Cool/NfdFileFilter/NfdFileFilter.h>.
/// `initial_folder` is the folder that the dialog window should open at. Leave empty to use `file_path` as the initial folder.
auto file_opening(
    const char*                         label,
    std::filesystem::path*              file_path,
    std::vector<nfdfilteritem_t> const& file_filters       = {},
    std::filesystem::path               initial_folder     = {},
    bool                                show_dialog_button = true
) -> bool;
auto file_saving(
    const char*                         label,
    std::filesystem::path*              file_path,
    std::vector<nfdfilteritem_t> const& file_filters       = {},
    std::filesystem::path               initial_folder     = {},
    bool                                show_dialog_button = true
) -> bool;

/// UI for a file path that shows the file and its folder on two separate lines.
/// `file_filters` is a set of filters for the file types that should be selectable. Something like { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } }. You can find predefined filters in <Cool/NfdFileFilter/NfdFileFilter.h>.
auto file_and_folder_opening(
    const char*                         label,
    std::filesystem::path*              path,
    std::vector<nfdfilteritem_t> const& file_filters = {}
) -> bool;

auto file_and_folder_saving(
    std::filesystem::path&              path,
    std::span<const char* const>        extensions,
    PathChecks const&                   path_checks,
    std::vector<nfdfilteritem_t> const& file_filters = {}
) -> bool;

void before_export_button();
void before_export_button(std::filesystem::path const& file_to_be_exported, PathChecks const& path_checks);

/// Equivalent to ImGui::Image except the image will be centered in the window
void image_centered(ImTextureID texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

/// A toggle that, when set to on, displays a menu on the side.
/// `submenu` is a function that calls the imgui widgets that should appear in the submenu, and returns true iff one of these widgets returned true.
/// returns true iff the toggle or a widget in the submenu was used this frame.
bool toggle_with_submenu(const char* label, bool* bool_p, std::function<bool()> const& submenu);

/// Like ImGui::BeginDisabled() + ImGui::EndDisabled(), but adds a message on hover
void disabled_if(bool condition_to_disable, const char* reason_to_disable, std::function<void()> const& widgets);
/// Like ImGui::BeginDisabled() + ImGui::EndDisabled(), but adds a message on hover
void disabled_if(std::optional<const char*> reason_to_disable, std::function<void()> const& widgets);
/// Like ImGui::BeginDisabled() + ImGui::EndDisabled(), but adds a message on hover
void disabled_if(std::optional<std::string> const& reason_to_disable, std::function<void()> const& widgets);

/// Hues are numbers from 0 to 1. 0 and 1 correspond to red.
auto hue_wheel(const char* label, float* hue) -> bool;

/// Renders `widget` with a colored background.
/// `widget` must be a function that draws some ImGui widgets.
void background(ImVec4 color, std::function<void()> const& widget);

/// Renders `widget` with a highlighted background.
/// `widget` must be a function that draws some ImGui widgets.
void highlight(std::function<void()> const& widget, float opacity = 1.f);

auto big_selectable(std::function<void()> const& widgets) -> bool;

/// Creates a clickable link that opens the given url in the user's default web browser.
void link(std::string_view url);

/// Creates a clickable link that opens the given url in the user's default web browser.
/// It will be rendered as `label`.
void link(std::string_view url, std::string_view label);

/// Brings attention to the given widget (highlight, bring window to front, scroll to right position, etc.).
/// `widget` must be a function that draws some ImGui widgets.
/// If `should_bring_attention` is false, just renders the widget as usual. This is a convenience that makes calling code nicer.
void bring_attention_if(bool should_bring_attention, std::function<void()> const& widget);

///
auto colored_collapsing_header(std::string_view name, Cool::Color const& color) -> bool;

///
auto toggle(const char* label, bool* v) -> bool;

/// Draws a vertical separator between two buttons, and puts them on the same line.
void join_buttons();

/// Just like ImGui::Begin() but the window will take the whole available space.
void begin_fullscreen(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);

/// Creates a button that is always in the bottom left corner of the current window.
/// Since you might want several buttons lining up, you have to specify the order in which
/// they should be drawn with `index`. The direction in which the several buttons will go
/// is controlled by `align_vertically`.
/// `is_enabled` allows you to treat your buttons as toggles and have them highlighted while `is_enabled` is true.
auto floating_button(const char* label, int index, bool align_vertically = true, bool is_enabled = false) -> bool;

void separator_text(std::string_view text);

auto input_text_multiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
    -> bool;

auto input_text_with_dropdown(const char* label, std::string* value, std::function<void(std::function<void(std::string const&)>)> const& for_each_dropdown_entry, ImGuiInputTextFlags flags = 0) -> bool;
auto dropdown(const char* label, std::string* value, std::function<void(std::function<void(std::string const&)>)> const& for_each_dropdown_entry) -> bool;

auto calc_custom_dropdown_input_width() -> float;

auto input_port(const char* label, int* port, ImGuiInputTextFlags = 0) -> bool;

void fill_layout(const char* str_id, float item_width, std::function<void(std::function<void()> const&)> const& callback);

void progress_bar(float fraction, const ImVec2& size_arg = ImVec2{-FLT_MIN, 0}, const char* overlay = nullptr);

} // namespace Cool::ImGuiExtras