#pragma once
#include "Cool/Dump/app_version.hpp"

namespace Cool {

struct SizeProportionalToScreen {
    float proportion{};
};
struct SizeInPixels {
    int pixels{};
};

using WindowSize = std::variant<
    SizeProportionalToScreen,
    SizeInPixels>;

struct WindowConfig {
    std::string title                       = fmt::format("{} {}", COOL_APP_NAME, app_version()); /// The title that will be displayed in the title bar of the window
    WindowSize  initial_width               = SizeProportionalToScreen{0.6f};                     /// Initial width of the window
    WindowSize  initial_height              = SizeProportionalToScreen{0.6f};                     /// Initial height of the window
    bool        cap_framerate_on_startup_if = true;                                               /// Whether the framerate should be capped or not (this can later be changed at runtime through window.cap_framerate_if())
    bool        maximize_on_startup_if      = true;                                               /// Whether the window will be maximized when the application starts.
    bool        hide_on_startup_if          = false;                                              /// Whether the window will be hidden when the application starts.
};

} // namespace Cool