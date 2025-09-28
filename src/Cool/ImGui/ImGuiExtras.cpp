#include "ImGuiExtras.h"
#include <Cool/File/File.h>
#include <Cool/Icons/Icons.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <smart/smart.hpp>
#include <wafl/wafl.hpp>
#include "Cool/File/PathChecks.hpp"
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtrasStyle.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/ImGui/markdown.h"
#include "Cool/Math/constants.h"
#include "Cool/UI Scale/ui_scale.hpp"
#include "Fonts.h"
#include "ImGuiExtrasStyle.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool::ImGuiExtras {

void help_marker(const char* text)
{
    ImGui::SameLine();
    help_marker_icon();
    if (ImGui::BeginItemTooltip())
    {
        help_marker_tooltip_content(text);
        ImGui::EndTooltip();
    }
}

void help_marker_icon()
{
    ImGui::TextDisabled(" " ICOMOON_INFO);
}

void help_marker_tooltip_content(const char* text)
{
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
}

bool angle_wheel(const char* label, float* value_p, int number_of_snaps, float snaps_offset, bool always_snap)
{
    float const thickness = 0.1f * ImGui::GetFontSize();
    float const radius    = 1.25f * ImGui::GetFontSize();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    //
    const ImGuiStyle& style       = ImGui::GetStyle();
    const float       line_height = ImGui::GetTextLineHeight();
    //
    ImVec2 p      = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(p.x + radius, p.y + radius);
    // Detect click
    ImGui::InvisibleButton(label, ImVec2(radius * 2.0f, radius * 2.0f));
    bool is_active  = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemHovered();

    if (is_active)
    {
        ImVec2 mp = ImGui::GetIO().MousePos;
        *value_p  = atan2f(center.y - mp.y, mp.x - center.x);
        if (always_snap || ImGui::GetIO().KeyShift)
        {
            const float slice = tau / static_cast<float>(number_of_snaps);
            *value_p          = std::floor((*value_p - snaps_offset) / slice + 0.5f) * slice + snaps_offset;
        }
    }

    float x2 = cosf(*value_p) * radius + center.x;
    float y2 = -sinf(*value_p) * radius + center.y;

    ImU32 col32 = ImGui::GetColorU32(
        is_active
            ? ImGuiCol_FrameBgActive
        : is_hovered
            ? ImGuiCol_FrameBgHovered
            : ImGuiCol_FrameBg
    );
    ImU32       col32line = ImGui::GetColorU32(ImGuiCol_SliderGrab);
    ImU32       col32text = ImGui::GetColorU32(ImGuiCol_Text);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCircleFilled(center, radius, col32);
    draw_list->AddLine(center, ImVec2(x2, y2), col32line, thickness);
    draw_list->AddText(ImVec2(p.x + radius * 2.0f + style.ItemInnerSpacing.y, p.y + radius - line_height * 0.5f), col32text, label);

    const ImGuiID id = window->GetID(label);
    if (is_active)
    {
        ImGui::MarkItemEdited(id);
    }
    return is_active;
}

auto angle_slider(const char* label, float* value_p) -> bool
{
    float angle_in_deg = *value_p / tau;
    angle_in_deg       = angle_in_deg - std::floor(angle_in_deg);
    angle_in_deg *= 360.f;

    if (ImGui::DragFloat(label, &angle_in_deg, 1.f, 0.f, 0.f, "%.0f deg"))
    {
        *value_p = fmod(angle_in_deg, 360.f) * tau / 360.f;
        return true;
    }
    return false;
}

bool direction_3d(const char* label, float* value_p1, float* value_p2)
{
    ImGui::BeginGroup(); // Group the two wheels so that things like IsItemDeactivatedAfterEdit() work properly
    ImGui::PushID(label);
    bool b = false;

    ImGui::Text("%s :", label);
    b |= angle_wheel("Angle Ground", value_p1);
    b |= angle_wheel("Angle Up", value_p2);

    ImGui::PopID();
    ImGui::EndGroup();
    return b;
}

void button_disabled(const char* label, const char* reason_for_disabling)
{
    disabled_if(true, reason_for_disabling, [&]() {
        ImGui::Button(label);
    });
}

auto colored_button(const char* label, float hue, const ImVec2& size) -> bool
{
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
    auto const is_pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return is_pressed;
}

auto colored_button(const char* label, Color color, const ImVec2& size) -> bool
{
    ImGui::PushStyleColor(ImGuiCol_Button, color.as_ImColor().Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color.brighter().as_ImColor().Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color.brighter().brighter().as_ImColor().Value);
    auto const is_pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return is_pressed;
}

bool button_with_icon(ImTextureID tex_id, const ImVec4& tint_color, const ImVec4& background_color, float button_width, float button_height, int frame_padding)
{
    return ImGui::ImageButton(tex_id, ImVec2(button_width, button_height), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f), frame_padding, background_color, tint_color);
}

void button_with_icon_disabled(ImTextureID tex_id, const char* reason_for_disabling, float button_width, float button_height, std::optional<float> frame_padding)
{
    const ImVec4 grey = ImVec4(0.35f, 0.35f, 0.35f, 1.f);
    image_framed(tex_id, ImVec2(button_width, button_height), {frame_padding, grey, ImVec4(0.f, 0.f, 0.f, 1.f), grey});
    ImGui::SetItemTooltip("%s", reason_for_disabling);
}

auto button_with_text_icon(const char* icon, ImDrawFlags flags) -> bool
{
    auto const size = ImGui::GetFrameHeight();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.4f * ui_scale());
    bool const b = ImGui::Button(icon, {size, size}, flags);
    ImGui::PopStyleVar();
    return b;
}

auto checkbox_button(const char* icon, bool* v) -> bool
{
    if (*v)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGuiExtras::GetStyle().checkbox_button);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiExtras::GetStyle().checkbox_button_hovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiExtras::GetStyle().checkbox_button_active);
    }
    else
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
    }
    bool const b = button_with_text_icon(icon);
    if (*v)
    {
        ImGui::PopStyleColor(3);
    }
    else
    {
        ImGui::PopStyleVar();
    }
    if (b)
        *v = !*v;
    return b;
}

auto close_button() -> bool
{
    return button_with_icon(
        Icons::close_button().imgui_texture_id(),
        ImVec4(0.9f, 0.9f, 0.9f, 1.f),
        ImVec4(0.5f, 0.2f, 0.2f, 1.f),
        11.f, 11.f
    );
}

void image_framed(ImTextureID tex_id, const ImVec2& size, image_framed_options const& o)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }

    ImGuiContext&     g     = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Default to using texture ID as ID. User can still push string/integer prefixes.
    // We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
    ImGui::PushID(tex_id);
    const ImGuiID id = window->GetID("#image");
    ImGui::PopID();
    const ImVec2 padding = o.frame_thickness ? ImVec2(*o.frame_thickness, *o.frame_thickness) : style.FramePadding;
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
    const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id))
    {
        return;
    }

    // Render
    const ImU32 frameCol = o.frame_color.w > 0.0f ? ImGui::GetColorU32(o.frame_color) : ImGui::GetColorU32(ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, frameCol, true, ImClamp(ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    ImGui::RenderFrame(image_bb.Min, image_bb.Max, ImGui::GetColorU32(o.background_color), true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (o.background_texture_id != nullptr)
        window->DrawList->AddImage(o.background_texture_id, image_bb.Min, image_bb.Max);
    window->DrawList->AddImage(tex_id, image_bb.Min, image_bb.Max, ImVec2(0, o.flip_y ? 0.f : 1.f), ImVec2(1, o.flip_y ? 1.f : 0.f), ImGui::GetColorU32(o.tint_color));
}

bool input_uint(const char* label, unsigned int* value_p)
{
    return ImGui::InputScalar(label, ImGuiDataType_U32, value_p, nullptr, nullptr, "%u");
}

bool slider_uint32(const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderScalar(label, ImGuiDataType_U32, v, &v_min, &v_max, format, flags);
}

void warning_text(const char* text)
{
    ImGui::TextColored(ImGuiNotify::get_style().color_warning, "%s", text);
}

bool begin_popup_context_menu_from_button(const char* label, ImGuiPopupFlags popup_flags)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    if (window->SkipItems)
    {
        return false;
    }
    ImGuiID id = label ? window->GetID(label) : GImGui->LastItemData.ID; // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
    IM_ASSERT(id != 0);                                                  // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
    if (ImGui::Button(label))
    {
        ImGui::OpenPopupEx(id, popup_flags);
    }
    return ImGui::BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

void invisible_wrapper_around_previous_line(const char* str_id)
{
    ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(0, 2 * ImGui::GetTextLineHeight()));
    ImGui::InvisibleButton(str_id, ImVec2(ImGui::GetWindowWidth(), 2 * ImGui::GetTextLineHeight()));
}

auto folder_dialog_button(
    std::filesystem::path*          out_path,
    File::folder_dialog_args const& args
) -> bool
{
    if (!button_with_text_icon(ICOMOON_FOLDER_OPEN))
        return false;

    auto const maybe_path = File::folder_dialog(args);
    if (!maybe_path)
        return false;

    *out_path = *maybe_path;
    return true;
}

auto file_opening_dialog_button(
    std::filesystem::path*        out_path,
    File::file_dialog_args const& args
) -> bool
{
    if (!button_with_text_icon(ICOMOON_FOLDER_OPEN))
        return false;

    auto const maybe_path = File::file_opening_dialog(args);
    if (!maybe_path)
        return false;

    *out_path = *maybe_path;
    return true;
}

auto file_saving_dialog_button(
    std::filesystem::path*        out_path,
    File::file_dialog_args const& args
) -> bool
{
    if (!button_with_text_icon(ICOMOON_FOLDER_OPEN))
        return false;

    auto const maybe_path = File::file_saving_dialog(args);
    if (!maybe_path)
        return false;

    *out_path = *maybe_path;
    return true;
}

template<typename Callable>
static auto folder_file_impl(const char* label, std::filesystem::path* path, bool show_dialog_button, Callable&& dialog_button) -> bool
{
    ImGui::PushID(path);
    ImGui::BeginGroup(); // This allows users to treat all these widgets as a single widget (when checking for hovering, activation, and stuff)
    bool b              = false;
    auto path_as_string = Cool::File::weakly_canonical(*path).string();
    ImGui::TextUnformatted(label);
    if (show_dialog_button)
    {
        ImGui::SameLine();
        b |= dialog_button();
    }
    ImGui::SameLine();
    if (ImGui::InputText("", &path_as_string))
    {
        b     = true;
        *path = std::filesystem::path{path_as_string};
    }
    ImGui::EndGroup();
    ImGui::PopID();
    return b;
}

auto folder(const char* label, std::filesystem::path* folder_path, bool show_dialog_button) -> bool
{
    return folder_file_impl(label, folder_path, show_dialog_button, [&]() {
        return ImGuiExtras::folder_dialog_button(folder_path, {.initial_folder = *folder_path});
    });
}

auto file_opening(const char* label, std::filesystem::path* file_path, std::vector<nfdfilteritem_t> const& file_filters, std::filesystem::path initial_folder, bool show_dialog_button) -> bool
{
    return folder_file_impl(label, file_path, show_dialog_button, [&]() {
        return ImGuiExtras::file_opening_dialog_button(file_path, {.file_filters = file_filters, .initial_folder = !initial_folder.empty() ? initial_folder : *file_path});
    });
}
auto file_saving(const char* label, std::filesystem::path* file_path, std::vector<nfdfilteritem_t> const& file_filters, std::filesystem::path initial_folder, bool show_dialog_button) -> bool
{
    return folder_file_impl(label, file_path, show_dialog_button, [&]() {
        return ImGuiExtras::file_saving_dialog_button(file_path, {.file_filters = file_filters, .initial_folder = !initial_folder.empty() ? initial_folder : *file_path});
    });
}

auto file_and_folder_opening(
    const char*                         label,
    std::filesystem::path*              path,
    std::vector<nfdfilteritem_t> const& file_filters
) -> bool
{
    bool b           = false;
    auto folder_path = File::without_file_name(*path);
    auto file_path   = File::file_name(*path);

    b |= file_opening((label + " (file)"s).c_str(), &file_path, file_filters, folder_path);
    b |= folder((label + " (folder)"s).c_str(), &folder_path, false);

    if (b)
        *path = folder_path / file_path;

    return b;
}

auto file_and_folder_saving(
    std::filesystem::path&              path,
    std::span<const char* const>        extensions,
    PathChecks const&                   path_checks,
    std::vector<nfdfilteritem_t> const& file_filters
) -> bool
{
    bool b = false;
    if (path.is_relative())
    {
        path = Cool::File::weakly_canonical(Path::project_folder().value_or(Path::user_data()) / path);
        b    = true;
    }

    auto folder              = File::without_file_name(path);
    auto file_with_extension = File::file_name(path);
    auto extension           = File::extension(path);

    b |= ImGuiExtras::file_saving("File", &file_with_extension, file_filters, folder, true /*Show dialog button*/);
    if (ImGuiExtras::folder("Folder", &folder))
    {
        b                   = true;
        file_with_extension = File::with_extension(File::find_available_name(folder, File::file_name_without_extension(path), extension, path_checks), extension);
    }

    const char* best_matching_extension = wafl::find_best_match(extensions, extension.string());
    if (ImGui::BeginCombo("Format", best_matching_extension))
    {
        for (const char* ext : extensions)
        {
            auto const is_selected = ext == extension;
            if (ImGui::Selectable(ext, is_selected))
            {
                b = true;
                file_with_extension.replace_extension(ext);
            }
            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (b)
        path = folder / file_with_extension;
    return b;
}

void before_export_button()
{
    ImGui::SeparatorText("");
}

void before_export_button(std::filesystem::path const& file_to_be_exported, PathChecks const& path_checks)
{
    before_export_button();

    auto const warnings = path_checks.compute_all_warnings(file_to_be_exported);
    ImGui::PushTextWrapPos();
    for (auto const& warning : warnings)
        ImGuiExtras::warning_text(Cool::icon_fmt(warning, ICOMOON_WARNING).c_str());
    ImGui::PopTextWrapPos();
}

void image_centered(ImTextureID texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
    auto const prev_pos = ImGui::GetCursorScreenPos();

    ImGui::SetCursorPos((ImGui::GetWindowSize() + ImVec2{0.f, ImGui::GetCurrentWindow()->TitleBarHeight()} - size) * 0.5f);
    ImGui::Image(texture_id, size, uv0, uv1, tint_col, border_col);

    ImGui::SetCursorScreenPos(prev_pos);
}

auto toggle_with_submenu(const char* label, bool* bool_p, std::function<bool()> const& submenu) -> bool
{
    bool was_used = false;
    ImGui::PushID(label);
    bool const menu_open = *bool_p;

    {
        auto const toggle_label = std::string{menu_open ? "" : label} + "###checkbox"; // We use the ### to make sure the id of the widget won't depend on the part of the label before the ###. This allows us to keep the same id when switching between "" and `label`. This is required for the animation of the toggle to play properly.
        was_used |= ImGuiExtras::toggle(toggle_label.c_str(), bool_p);
    }

    if (menu_open)
    {
        ImGui::SameLine();
        if (ImGui::BeginMenu(label, true, ImGui::GetStyle().FramePadding.y))
        {
            if (ImGui::IsItemClicked())
                *bool_p = !bool_p;
            was_used |= submenu();
            ImGui::EndMenu();
        }
    }

    ImGui::PopID();
    return was_used;
}

void disabled_if(bool condition_to_disable, const char* reason_to_disable, std::function<void()> const& widgets)
{
    disabled_if(condition_to_disable ? std::make_optional(reason_to_disable) : std::nullopt, widgets);
}

void disabled_if(std::optional<std::string> const& reason_to_disable, std::function<void()> const& widgets)
{
    disabled_if(reason_to_disable.has_value(), reason_to_disable.has_value() ? reason_to_disable->c_str() : "", widgets);
}

void disabled_if(std::optional<const char*> reason_to_disable, std::function<void()> const& widgets)
{
    if (reason_to_disable.has_value())
    {
        ImGui::BeginGroup();
        ImGui::BeginDisabled(true);

        widgets();

        ImGui::EndDisabled();
        ImGui::EndGroup();
        if (/*!reason_to_disable.empty()*/ (*reason_to_disable)[0] != '\0')
            ImGui::SetItemTooltip("%s", reason_to_disable.value());
    }
    else
    {
        ImGui::BeginGroup();

        widgets();

        ImGui::EndGroup();
    }
}

auto hue_wheel(const char* label, float* hue) -> bool
{
    ImGuiContext&      g      = *GImGui;
    const ImGuiWindow& window = *ImGui::GetCurrentWindow();
    if (window.SkipItems)
        return false;

    ImDrawList&       draw_list = *window.DrawList;
    const ImGuiStyle& style     = g.Style;

    g.NextItemData.ClearFlags();

    ImGui::PushID(label);
    ImGui::BeginGroup();

    // Setup
    const ImVec2 widget_pos = ImGui::GetCursorScreenPos();

    float const radius          = 1.25f * ImGui::GetFontSize();
    float const wheel_thickness = radius * .5f;
    float const wheel_r_outer   = radius;
    float const wheel_r_inner   = wheel_r_outer - wheel_thickness;
    const auto  wheel_center    = ImVec2{
        widget_pos.x + wheel_r_outer,
        widget_pos.y + wheel_r_outer
    };

    bool        value_changed = false;
    const float initial_hue   = *hue;

    // Hue wheel
    ImGui::InvisibleButton("Hue", ImVec2(wheel_r_outer * 2.f + style.ItemInnerSpacing.x, wheel_r_outer * 2.f));
    if (ImGui::IsItemActive())
    {
        const ImVec2 initial_off   = g.IO.MouseClickedPos[0] - wheel_center;
        const ImVec2 current_off   = g.IO.MousePos - wheel_center;
        const float  initial_dist2 = ImLengthSqr(initial_off);
        if (initial_dist2 >= (wheel_r_inner - 1) * (wheel_r_inner - 1) && initial_dist2 <= (wheel_r_outer + 1) * (wheel_r_outer + 1))
        {
            // Interaction with Hue wheel
            *hue = ImAtan2(current_off.y, current_off.x) / IM_PI * 0.5f;
            if (*hue < 0.0f)
                *hue += 1.0f;
            value_changed = true;
        }
    }

    const int   style_alpha8    = IM_F32_TO_INT8_SAT(style.Alpha);
    const ImU32 col_white       = IM_COL32(255, 255, 255, style_alpha8);
    const ImU32 col_midgrey     = IM_COL32(128, 128, 128, style_alpha8);
    const ImU32 col_hues[6 + 1] = {IM_COL32(255, 0, 0, style_alpha8), IM_COL32(255, 255, 0, style_alpha8), IM_COL32(0, 255, 0, style_alpha8), IM_COL32(0, 255, 255, style_alpha8), IM_COL32(0, 0, 255, style_alpha8), IM_COL32(255, 0, 255, style_alpha8), IM_COL32(255, 0, 0, style_alpha8)};

    ImVec4 hue_color_f(1, 1, 1, 0);
    ImU32  hue_color32 = ImGui::ColorConvertFloat4ToU32(hue_color_f);

    // Render Hue Wheel
    const float aeps            = 0.5f / wheel_r_outer; // Half a pixel arc length in radians (2pi cancels out).
    const int   segment_per_arc = ImMax(4, static_cast<int>(wheel_r_outer) / 3);
    for (int n = 0; n < 6; n++)
    {
        const auto  nn             = static_cast<float>(n);
        const float a0             = nn / 6.0f * 2.0f * IM_PI - aeps;
        const float a1             = (nn + 1.0f) / 6.0f * 2.0f * IM_PI + aeps;
        const int   vert_start_idx = draw_list.VtxBuffer.Size;
        draw_list.PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer) * 0.5f, a0, a1, segment_per_arc);
        draw_list.PathStroke(col_white, 0, wheel_thickness);
        const int vert_end_idx = draw_list.VtxBuffer.Size;

        // Paint colors over existing vertices
        ImVec2 gradient_p0(wheel_center.x + ImCos(a0) * wheel_r_inner, wheel_center.y + ImSin(a0) * wheel_r_inner);
        ImVec2 gradient_p1(wheel_center.x + ImCos(a1) * wheel_r_inner, wheel_center.y + ImSin(a1) * wheel_r_inner);
        ImGui::ShadeVertsLinearColorGradientKeepAlpha(&draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col_hues[n], col_hues[n + 1]);
    }

    // Render Cursor + preview on Hue Wheel
    float  cos_hue_angle = ImCos(*hue * 2.0f * IM_PI);
    float  sin_hue_angle = ImSin(*hue * 2.0f * IM_PI);
    ImVec2 hue_cursor_pos(wheel_center.x + cos_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f, wheel_center.y + sin_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f);
    float  hue_cursor_rad      = value_changed ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
    int    hue_cursor_segments = ImClamp(static_cast<int>(hue_cursor_rad / 1.4f), 9, 32) * 10;
    draw_list.AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32, hue_cursor_segments);
    draw_list.AddCircle(hue_cursor_pos, hue_cursor_rad + 1, col_midgrey, hue_cursor_segments);
    draw_list.AddCircle(hue_cursor_pos, hue_cursor_rad, col_white, hue_cursor_segments);
    draw_list.AddText(ImVec2(wheel_center.x + wheel_r_outer + style.ItemInnerSpacing.x, wheel_center.y - style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    ImGui::EndGroup();

    if (initial_hue == *hue)
    {
        value_changed = false;
    }

    if (value_changed)
    {
        ImGui::MarkItemEdited(g.LastItemData.ID);
    }

    ImGui::PopID();

    return value_changed;
}

static auto ImU32_from_ImVec4(ImVec4 color) -> ImU32
{
    return ImGui::GetColorU32(IM_COL32(
        color.x * 255.f,
        color.y * 255.f,
        color.z * 255.f,
        color.w * 255.f
    ));
}

static auto highlight_color(float opacity) -> ImVec4
{
    auto col = GetStyle().highlight_items;
    col.w    = opacity;
    return col;
}

void background(ImVec4 color, std::function<void()> const& widget)
{
    ImDrawList& draw_list = *ImGui::GetWindowDrawList();
    draw_list.ChannelsSplit(2);      // Allows us to draw the background rectangle behind the widget, even though the widget is drawn first.
    draw_list.ChannelsSetCurrent(1); // We must draw them in that order because we need to know the height of the widget before drawing the rectangle.
    ImVec2 const rectangle_start_pos = ImGui::GetCursorScreenPos() - ImVec2{0.f, ImGui::GetStyle().ItemSpacing.y / 2.f};

    widget();

    auto const rectangle_end_pos = ImVec2{
        rectangle_start_pos.x + ImGui::GetContentRegionAvail().x,
        ImGui::GetCursorScreenPos().y - ImGui::GetStyle().ItemSpacing.y / 2.f,
    };
    draw_list.ChannelsSetCurrent(0);
    draw_list.AddRectFilled(
        rectangle_start_pos,
        rectangle_end_pos,
        ImU32_from_ImVec4(color)
    );
    draw_list.ChannelsMerge();
}

void highlight(std::function<void()> const& widget, float opacity)
{
    background(
        highlight_color(opacity),
        widget
    );
}

auto big_selectable(std::function<void()> const& widgets) -> bool
{
    ImDrawList& draw_list = *ImGui::GetWindowDrawList();
    draw_list.ChannelsSplit(2);      // Allows us to draw the background rectangle behind the widgets, even though the widgets are drawn first.
    draw_list.ChannelsSetCurrent(1); // We must draw them in that order because we need to know the height of the widgets before drawing the rectangle.
    ImVec2 const rectangle_start_pos = ImGui::GetCursorScreenPos() - ImVec2{0.f, ImGui::GetStyle().ItemSpacing.y / 2.f};

    widgets();

    auto const rectangle_end_pos = ImVec2{
        rectangle_start_pos.x + ImGui::GetContentRegionAvail().x,
        ImGui::GetCursorScreenPos().y - ImGui::GetStyle().ItemSpacing.y / 2.f,
    };

    ImGui::SetCursorScreenPos(rectangle_start_pos);
    bool const pressed = ImGui::InvisibleButton("", rectangle_end_pos - rectangle_start_pos);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        draw_list.ChannelsSetCurrent(0);
        draw_list.AddRectFilled(
            rectangle_start_pos,
            rectangle_end_pos,
            ImGui::GetColorU32(ImGui::IsItemActive() ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered)
        );
    }
    draw_list.ChannelsMerge();
    return pressed;
}

void link(std::string_view url)
{
    link(url, url);
}

void link(std::string_view url, std::string_view label)
{
    auto const pos = ImGui::GetCursorPos();
    ImGui::Dummy({ImGui::CalcTextSize(label.data())});
    ImGui::SetCursorPos(pos);
    ImGuiExtras::markdown(fmt::format("[{}]({})", label, url));
}

void bring_attention_if(bool should_bring_attention, std::function<void()> const& widget)
{
    if (should_bring_attention)
    {
        ImGui::SetWindowToFront();
        ImGuiExtras::highlight(widget, 1.f);
        ImGui::SetScrollHereX(0.f);
        ImGui::SetScrollHereY(0.f);
    }
    else
    {
        widget();
    }
}

static auto glm_to_imvec4(glm::vec3 const& v)
{
    return ImVec4(v.x, v.y, v.z, 1.f);
}

auto colored_collapsing_header(std::string_view name, Cool::Color const& color) -> bool
{
    auto const base_color   = color.as_sRGB();
    auto const bright_color = color.brighter().as_sRGB();

    ImGui::PushStyleColor(ImGuiCol_Header, glm_to_imvec4(base_color));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, glm_to_imvec4(bright_color));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, glm_to_imvec4(bright_color));
    ImGui::PushStyleColor(ImGuiCol_Text, get_text_color(color).as_ImColor().Value);

    auto const b = ImGui::CollapsingHeader(name.data());
    ImGui::PopStyleColor(4);
    return b;
}

auto toggle(const char* label, bool* v) -> bool
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext&     g          = *GImGui;
    const ImGuiStyle& style      = g.Style;
    const ImGuiID     id         = window->GetID(label);
    const ImVec2      label_size = ImGui::CalcTextSize(label, nullptr, true);
    const ImVec2      p          = ImGui::GetCursorScreenPos();

    const float  height = ImGui::GetFrameHeight();
    const float  width  = height * 1.55f;
    const ImVec2 pos    = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool       hovered, held; // NOLINT
    const bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);
    if (pressed)
    {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
    }

    const ImRect check_bb(pos, pos + ImVec2(width, height));
    ImGui::RenderNavHighlight(total_bb, id);

    float const radius  = height * 0.5f;
    float const outline = 0.125f * ImGui::GetFontSize();
    float       t       = *v ? 1.f : 0.f;

    static constexpr float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == id && g.LastActiveIdTimer < ANIM_SPEED)
    {
        float const t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t                  = *v ? t_anim : (1.f - t_anim);
    }

    ImU32 const col_bg = ImGui::IsItemHovered()
                             ? ImGui::GetColorU32(ImLerp(GetStyle().toggle_bg_hovered, GetStyle().toggle_hovered, t))
                             : ImGui::GetColorU32(ImLerp(GetStyle().toggle_bg, GetStyle().toggle, t));

    ImDrawList* const draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius, ImGui::GetColorU32(hovered ? GetStyle().toggle_hovered : GetStyle().toggle));
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - outline, ImGui::GetColorU32(hovered ? GetStyle().toggle_bg_hovered : GetStyle().toggle_bg));

    ImVec2 const label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
    if (label_size.x > 0.0f)
        ImGui::RenderText(label_pos, label);

    return pressed;
}

void join_buttons()
{
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Separator, ImGuiExtras::GetStyle().buttons_separator);
    ImGui::PopStyleColor();
    ImGui::SameLine();
}

void begin_fullscreen(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
#else
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
    ImGui::Begin(
        name,
        p_open,
        flags
            | ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus
    );
}

auto floating_button(const char* label, int index, bool align_vertically, bool is_enabled) -> bool
{
    auto const spacing = [&]() { // Immediately-invoked lambda
        auto const size        = ImGui::GetFrameHeight();
        auto       spacing_tmp = ImGui::GetStyle().WindowPadding + ImVec2{size, size};
        auto const offset      = static_cast<float>(index) * (size + GetStyle().floating_buttons_spacing);
        if (align_vertically)
            spacing_tmp.y += offset;
        else
            spacing_tmp.x += offset;
        return spacing_tmp;
    }();

    auto const prev_pos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorPos(ImGui::GetWindowSize() - spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.8f * ui_scale());
    // clang-format off
    ImGui::PushStyleColor(ImGuiCol_Button,        is_enabled ? GetStyle().floating_button_enabled         : GetStyle().floating_button);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, is_enabled ? GetStyle().floating_button_hovered_enabled : GetStyle().floating_button_hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  is_enabled ? GetStyle().floating_button_active_enabled  : GetStyle().floating_button_active);
    // clang-format on
    bool const b = button_with_text_icon(label);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::SetCursorScreenPos(prev_pos);
    return b;
}

void separator_text(std::string_view text)
{
    ImGui::PushFont(Font::bold());
    ImGui::SeparatorText(text.data());
    ImGui::PopFont();
}

auto input_text_multiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    -> bool
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().ChildRounding);
    bool const res = ImGui::InputTextMultiline(label, str, size, flags, callback, user_data);
    ImGui::PopStyleVar();
    return res;
}

auto input_text_with_dropdown(const char* label, std::string* value, std::function<void(std::function<void(std::string const&)>)> const& for_each_dropdown_entry, ImGuiInputTextFlags flags) -> bool
{
    bool b = false;
    ImGui::PushID(label);
    ImGui::SetNextItemWidth(calc_custom_dropdown_input_width());
    b |= ImGui::InputText("", value, flags, nullptr, nullptr, ImDrawFlags_RoundCornersLeft);
    ImGui::SameLine(0.f, 0.f);
    if (ImGui::BeginCombo(label, value->c_str(),
                          ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft, // Draw just the arrow of the dropdown
                          ImDrawFlags_RoundCornersRight))
    {
        for_each_dropdown_entry([&](std::string const& combo_content) {
            bool const is_selected = *value == combo_content;
            if (ImGui::Selectable(combo_content.c_str(), is_selected))
            {
                *value = combo_content;
                b      = true;
            }
            if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                ImGui::SetItemDefaultFocus();
        });
        ImGui::EndCombo();
    }
    ImGui::PopID();
    return b;
}

auto dropdown(const char* label, std::string* value, std::function<void(std::function<void(std::string const&)>)> const& for_each_dropdown_entry) -> bool
{
    bool b = false;
    ImGui::PushID(label);
    if (ImGui::BeginCombo(label, value->c_str()))
    {
        for_each_dropdown_entry([&](std::string const& combo_content) {
            bool const is_selected = *value == combo_content;
            if (ImGui::Selectable(combo_content.c_str(), is_selected))
            {
                *value = combo_content;
                b      = true;
            }
            if (is_selected) // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                ImGui::SetItemDefaultFocus();
        });
        ImGui::EndCombo();
    }
    ImGui::PopID();
    return b;
}

auto calc_custom_dropdown_input_width() -> float
{
    return ImGui::CalcItemWidth() - ImGui::GetFrameHeight();
}

auto input_port(const char* label, int* port, ImGuiInputTextFlags flags) -> bool
{
    ImGui::SetNextItemWidth(ImGui::CalcTextSize("00000").x + ImGui::GetStyle().FramePadding.x); // Enough width for any 5-digit number.
    return ImGui::InputInt(label, port, -1, -1, ImGuiInputTextFlags_AutoSelectAll | flags);
}

void fill_layout(const char* str_id, float item_width, std::function<void(std::function<void()> const&)> const& callback)
{
    auto const available_width = ImGui::GetContentRegionAvail().x;
    auto const items_per_row   = smart::keep_above(1, static_cast<int>(std::floor(available_width / item_width)));
    if (ImGui::BeginTable(str_id, items_per_row))
    {
        callback([]() { ImGui::TableNextColumn(); });
        ImGui::EndTable();
    }
    ImGui::SameLine();
}

void progress_bar(float fraction, const ImVec2& size_arg, const char* overlay)
{
    ImGui::PushFont(Font::monospace());
    ImGui::ProgressBar(fraction, size_arg, overlay);
    ImGui::PopFont();
}

} // namespace Cool::ImGuiExtras