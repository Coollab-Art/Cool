#include "View.h"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Input/MouseCoordinates.h"
#include "Cool/Log/message_console.hpp"
#include "Cool/UserSettings/UserSettings.h"
#include "Cool/Window/set_main_window_fullscreen.hpp"
#include "img/src/SizeU.h"

namespace Cool {

void View::check_for_fullscreen_toggle()
{
    // if (!_is_output_view)
    //     return;
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
        return;

    if (ImGui::IsKeyChordPressed(ImGuiKey_F10))
        toggle_fullscreen();

    if (ImGui::IsKeyChordPressed(ImGuiKey_Escape))
        exit_fullscreen();
}

void View::toggle_fullscreen()
{
    if (!user_settings().enable_multi_viewport) // If this imgui window cannot escape the main window, we need the main window itself to go fullscreen if we want this imgui window to be able to take the full screen
        toggle_main_window_fullscreen();
    _need_to_toggle_fullscreen = true; // We delay it by 1 frame so that the main window has time to go fullscreen, which we need to know the size that our imgui window needs to take to be fullscreen
}

void View::exit_fullscreen()
{
    if (!user_settings().enable_multi_viewport) // If this imgui window cannot escape the main window, we need the main window itself to go fullscreen if we want this imgui window to be able to take the full screen
        set_main_window_fullscreen(false);
    ImGui::ExitWindowFullscreen();
}

void View::imgui_window(ViewWindowParams const& params)
{
    if (_is_open != _was_open_last_frame)
    {
        if (_is_open)
            params.on_open();
        else
            params.on_close();
        _was_open_last_frame = _is_open;
    }
    if (!_is_open)
    {
        _window_size.reset();
        _window_is_hovered = false;
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f}); // TODO add a parameter in the UI to control the padding specifically for the views
    {                                                             // Begin window, maybe in fullscreen
        bool* const            p_open = _is_closable ? &_is_open : nullptr;
        ImGuiWindowFlags const flags  = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | (_is_output_view ? ImGuiWindowFlags_NoTitleBar : 0);
        if (params.fullscreen)
            ImGuiExtras::begin_fullscreen(_name.c_str(), p_open, flags);
        else
            ImGui::Begin(_name.c_str(), p_open, flags);
    }
    ImGui::PopStyleVar();
    if (_need_to_toggle_fullscreen)
    {
        ImGui::ToggleWindowFullscreen();
        _need_to_toggle_fullscreen = false;
    }
    check_for_fullscreen_toggle();
    ImGui::PushStyleColor(ImGuiCol_NavCursor, {0.f, 0.f, 0.f, 0.f});                                      // Hack because when escaping view's fullscreen with the ESCAPE key it gets nav-highlighted.
    ImGui::BeginChild("##ChildWindow", {0.f, 0.f}, false, _is_output_view ? 0 : ImGuiWindowFlags_NoMove); // Hack to emulate `ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;` for this window only. Since we can drag a camera in a View we don't want the window to move at the same time.
    {
        store_window_size();
        store_window_position();
        _window_is_hovered = ImGui::IsWindowHovered();

        display_image(get_image_texture_id(), get_image_size());

        _gizmos.render(*this);
        _accepts_mouse_events = !params.extra_widgets(); // When widgets are used, don't dispatch events on the View.
    }
    ImGui::EndChild();
    if (_is_output_view)
    {
        if (ImGui::BeginPopupContextItem("##close"))
        {
            if (ImGui::Button("Toggle Fullscreen (F10)"))
                toggle_fullscreen();
            if (ImGui::Button("Close window"))
                close();
            ImGui::EndPopup();
        }
    }
    ImGui::PopStyleColor();
    ImGui::End();
}

void View::imgui_open_close_toggle()
{
    ImGuiExtras::toggle(_name.c_str(), &_is_open);
}

void View::open()
{
    _is_open = true;
}

void View::close()
{
    _is_open = false;
}

void View::toggle_open_close()
{
    _is_open = !_is_open;
}

auto View::mouse_is_in_view() const -> bool
{
    return _window_is_hovered && _window_size;
}

auto View::to_view_coordinates(ImGuiCoordinates const pos, bool should_apply_translation) const -> ViewCoordinates
{
    if (!_window_size)
        return ViewCoordinates{};
    auto const window_size = glm::vec2{_window_size->width(), _window_size->height()};
    auto const img_size    = img::fit_into(*_window_size, get_image_size());

    auto res = glm::vec2{pos};
    if (should_apply_translation)
        res -= _window_position + window_size / 2.f;
    res /= img_size.height() / 2.f;
    res.y *= -1.f;

    return ViewCoordinates{res};
}

auto View::to_imgui_coordinates(ViewCoordinates pos) const -> ImGuiCoordinates
{
    if (!_window_size)
        return ImGuiCoordinates{};
    auto const window_size = glm::vec2{_window_size->width(), _window_size->height()};
    auto const img_size    = img::fit_into(*_window_size, get_image_size());
    auto       res         = glm::vec2{pos};

    res.y *= -1.f;
    res *= img_size.height() / 2.f;
    res += _window_position + window_size / 2.f;

    return ImGuiCoordinates{res};
}

void View::dispatch_mouse_move_event(MouseMoveEvent<ImGuiCoordinates> const& event)
{
    if (!_accepts_mouse_events)
        return;
    auto const pos   = to_view_coordinates(event.position, /*should_apply_translation=*/true);
    auto const delta = to_view_coordinates(event.delta, /*should_apply_translation=*/false);
    _mouse_event_dispatcher.drag().dispatch_mouse_move_event({pos, delta});
    _mouse_event_dispatcher.move_event().dispatch({pos, delta});

    if (DebugOptions::log_mouse_position_in_view())
    {
        message_console().send(
            _log_position_message_id,
            Message{
                .type    = MessageType::Info,
                .title   = _name,
                .content = fmt::format("Mouse at ({:.2f}, {:.2f})", pos.x, pos.y),
            }
        );
    }
}

void View::dispatch_mouse_scroll_event(MouseScrollEvent<ImGuiCoordinates> const& event)
{
    if (!_accepts_mouse_events)
        return;
    if (!mouse_is_in_view())
        return;

    _mouse_event_dispatcher.scroll_event().dispatch({
        .position = to_view_coordinates(event.position),
        .dx       = event.dx,
        .dy       = event.dy,
    });
}

void View::dispatch_mouse_button_event(MouseButtonEvent<ImGuiCoordinates> const& event)
{
    if (!_accepts_mouse_events)
        return;
    auto const pos       = to_view_coordinates(event.position);
    auto const new_event = MouseButtonEvent<ViewCoordinates>{
        .position = pos,
        .button   = event.button,
        .action   = event.action,
    };
    _mouse_event_dispatcher.drag().dispatch_mouse_button_event(new_event, mouse_is_in_view());
    if (mouse_is_in_view())
    {
        _mouse_event_dispatcher.button_event().dispatch(new_event);
    }
}

void View::store_window_size()
{
    const auto size = ImGui::GetContentRegionAvail();
    if (size.x >= 1.f && size.y >= 1.f)
    {
        const auto new_size = std::make_optional(img::Size{
            static_cast<img::Size::DataType>(size.x),
            static_cast<img::Size::DataType>(size.y),
        });

        _window_size = new_size;
    }
    else
    {
        _window_size.reset();
    }
}

void View::store_window_position()
{
    _window_position = ImGui::GetCursorScreenPos();
}

static auto as_imvec2(img::SizeT<float> size) -> ImVec2
{
    return {size.width(), size.height()};
}

void View::display_image(ImTextureID image_texture_id, img::Size image_size)
{
    if (!_window_size.has_value())
        return;

    auto const size       = img::fit_into(*_window_size, image_size);
    _has_vertical_margins = img::aspect_ratio(size) <= img::aspect_ratio(*_window_size);

    ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 0.f);
    // Alpha checkerboard background
    ImGuiExtras::image_centered(_checkerboard_texture.get(img::Size{size}).imgui_texture_id(), as_imvec2(size));
    // Actual image. It needs to use straight alpha as this is what ImGui expects.
    ImGuiExtras::image_centered(image_texture_id, as_imvec2(size));
    ImGui::PopStyleVar();
}

auto View::aspect_ratio() const -> float
{
    if (!_window_size.has_value())
        return 1.f;
    return img::aspect_ratio(*_window_size);
}

} // namespace Cool