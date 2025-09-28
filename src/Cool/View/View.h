#pragma once
#include "Cool/CheckerboardTexture/CheckerboardTexture.hpp"
#include "Cool/Input/MouseCoordinates.h"
#include "Cool/Input/MouseEventDispatcher.h"
#include "Cool/Log/MessageId.h"
#include "Cool/View/GizmoManager.h"
#include "GizmoManager.h"

namespace Cool {

struct ViewWindowParams {
    bool fullscreen = false;
    /// When this returns true, mouse events won't be forwarded to the View.
    std::function<bool()> extra_widgets = []() {
        return false;
    };
    std::function<void()> on_open{[] {
    }};
    std::function<void()> on_close{[] {
    }};
};

struct ViewCreationParams {
    std::string_view name{};
    bool             is_output_view{false}; /// Makes it with no title bar, and tweaks a few behaviours to make this work nicely
    bool             is_closable{false};
    bool             start_open{true};
};

/// A view is a window displaying an image.
/// You can subscribe to events.
/// You can use one of the various subclasses to decide where your image will come from.
class View {
public:
    explicit View(ViewCreationParams const& p)
        : _name{p.name}
        , _is_closable{p.is_closable}
        , _is_open{p.start_open}
        , _is_output_view{p.is_output_view}
    {
        _mouse_event_dispatcher.drag()
            .subscribe({
                .on_start  = [&](auto&& event) { return _gizmos.on_drag_start(event, *this); },
                .on_update = [&](auto&& event) { _gizmos.on_drag_update(event); },
                .on_stop   = [&](auto&& event) { _gizmos.on_drag_stop(event); },
            });
    }

    virtual ~View()                      = default;
    View(View const&)                    = delete;
    auto operator=(View const&) -> View& = delete;
    View(View&&)                         = delete;
    auto operator=(View&&) -> View&      = delete;

    /// Displays the given texture in an ImGui window
    void imgui_window(ViewWindowParams const& = {});
    void imgui_open_close_toggle();

    void open();
    void close();
    void toggle_open_close();

    /// Returns the size of the window.
    /// /!\ This is not the size of the image in the View, this is the size of the full window, including the potential margins around the image.
    auto window_size() const -> std::optional<img::Size> { return _window_size; }

    auto to_view_coordinates(ImGuiCoordinates, bool should_apply_translation = true) const -> ViewCoordinates;
    auto to_imgui_coordinates(ViewCoordinates) const -> ImGuiCoordinates;
    auto mouse_is_in_view() const -> bool;

    auto mouse_events() -> auto& { return _mouse_event_dispatcher; }

    /// You need to push your gizmos every frame when you want them to appear.
    /// Must be pushed before `imgui_window()`.
    auto gizmos_manager() -> GizmoManager& { return _gizmos; }

    auto has_vertical_margins() const -> bool { return _window_size ? _has_vertical_margins : false; }
    auto is_open() const -> bool { return _is_open; }
    auto aspect_ratio() const -> float;

private: /// Child classes need to implement these functions in order for us to display their image in the View.
    friend class ForwardingOrTextureView;
    virtual auto get_image_texture_id() const -> ImTextureID = 0;
    virtual auto get_image_size() const -> img::Size         = 0;

private:
    friend class AppManager;
    void dispatch_mouse_move_event(MouseMoveEvent<ImGuiCoordinates> const& event);
    void dispatch_mouse_scroll_event(MouseScrollEvent<ImGuiCoordinates> const& event);
    void dispatch_mouse_button_event(MouseButtonEvent<ImGuiCoordinates> const& event);

    void on_frame_end() { _gizmos.on_frame_end(); }

private:
    void store_window_size();
    void store_window_position();
    void display_image(ImTextureID image_texture_id, img::Size image_size);
    void check_for_fullscreen_toggle() const;

private:
    std::string                           _name;
    bool                                  _is_closable;
    bool                                  _is_open;
    bool                                  _is_output_view;
    bool                                  _was_open_last_frame{false};
    bool                                  _window_is_hovered{false};
    std::optional<img::Size>              _window_size{std::nullopt}; // Can be nullopt when the window is closed
    ImGuiCoordinates                      _window_position{};
    MouseEventDispatcher<ViewCoordinates> _mouse_event_dispatcher;
    CheckerboardTexture                   _checkerboard_texture{};
    bool                                  _accepts_mouse_events{true};
    GizmoManager                          _gizmos{};

    MessageId _log_position_message_id{};

    bool _has_vertical_margins{false};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Is open", _is_open)
        );
    }
};

} // namespace Cool