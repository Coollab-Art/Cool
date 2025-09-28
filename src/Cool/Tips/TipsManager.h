#pragma once
#include <chrono>
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/Serialization/Json.hpp"

namespace Cool {

using Tips = std::span<const char* const>;

/// A class that manages the "Did you know" modal. It is used to display tips to the user at the opening of the software.
/// Also it is possible to display all the tips at once.
class TipsManager {
public:
    void open_one_tip_window();
    void open_all_tips_window();
    void imgui_show_one_tip(Tips);
    void imgui_windows(Tips);

    void on_app_shutdown();

private:
    void open_ifn();
    void prepare_tip(int difference);
    void prepare_next_tip();
    void prepare_previous_tip();
    auto get_current_tip(Tips all_tips) -> const char*;

private:
    friend void test_tips(Cool::TipsManager&); // To display the internal values like remaining time before opening.

    int         _current_tip_index{-1}; // Start at -1 so that the first tip will be at index 0 (because we increment before opening the popup).
    bool        _show_all_tips{false};
    ImGuiWindow _window{Cool::icon_fmt("Did you know?", ICOMOON_BUBBLE), {.is_modal = true}};

    bool                                  _app_has_just_been_opened{true};
    std::chrono::steady_clock::time_point _time_when_app_was_last_closed{};

private:
    friend auto from_json(nlohmann::json const& json, TipsManager& tips)
    {
        Cool::json_get(json, "Current tip", tips._current_tip_index);
        Cool::json_get(json, "Wants to show all tips", tips._show_all_tips);
        int64_t milliseconds{};
        Cool::json_get(json, "Time when app was last closed", milliseconds);
        tips._time_when_app_was_last_closed = std::chrono::steady_clock::time_point{std::chrono::milliseconds(milliseconds)};
    }
    friend auto to_json(nlohmann::json& json, TipsManager const& tips)
    {
        Cool::json_set(json, "Current tip", tips._current_tip_index);
        Cool::json_set(json, "Wants to show all tips", tips._show_all_tips);
        Cool::json_set(json, "Time when app was last closed", std::chrono::duration_cast<std::chrono::milliseconds>(tips._time_when_app_was_last_closed.time_since_epoch()).count());
    }
};

} // namespace Cool
