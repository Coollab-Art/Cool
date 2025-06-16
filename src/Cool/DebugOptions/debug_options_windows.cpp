#include "debug_options_windows.h"
#include "Cool/ImGui/ColorThemes.h"
#include "Cool/ImGui/imgui_show_all_icons.hpp"
#include "Cool/ImGui/test_markdown_formatting.h"
#include "Cool/Log/TestMessageConsole.h"
#include "Cool/Midi/MidiManager.h"
#include "Cool/TextureSource/TextureLibrary_Image.h"
#include "Cool/TextureSource/TextureLibrary_Video.h"
#include "Cool/Tips/test_tips.h"
#include "Cool/Variables/TestPresets.h"
#include "Cool/Variables/TestVariables.h"
#include "DebugOptions.h"
#include "TestNotification.hpp"
#include "TestTasks.hpp"
#include "frame_time_stopwatch.hpp"

namespace Cool {

void debug_options_windows(TipsManager* tips_manager, Window& main_window)
{
    DebugOptions::show_framerate_window([&] {
        main_window.imgui_cap_framerate();
        frame_time_stopwatch().imgui_plot();
    });

    if (DebugOptions::show_imgui_demo_window())
    {
        bool b = DebugOptions::show_imgui_demo_window();
        ImGui::ShowDemoWindow(&b);
        DebugOptions::Set::show_imgui_demo_window(b);
    }

    DebugOptions::test_all_variable_widgets__window(&test_variables);
    DebugOptions::empty_window([] {});

    DebugOptions::texture_library_debug_view([&] {
        ImGui::SeparatorText("Image");
        TextureLibrary_Image::instance().imgui_debug_view();
        ImGui::SeparatorText("Video");
        TextureLibrary_Video::instance().imgui_debug_view();
    });

    DebugOptions::test_message_console__window([]() {
        static auto test_message_console = TestMessageConsole{};
        test_message_console.imgui();
    });

    DebugOptions::test_notifications__window([]() {
        static auto test_notification = TestNotification{};
        test_notification.imgui();
    });

    DebugOptions::test_tasks__window([]() {
        static auto test_tasks = TestTasks{};
        test_tasks.imgui();
    });

    DebugOptions::test_presets__window([]() {
        static auto test_presets = TestPresets{};
        test_presets.imgui();
    });

    DebugOptions::test_markdown_formatting_window([]() {
        test_markdown_formatting();
    });

    DebugOptions::emulate_midi_keyboard([]() {
        midi_manager().imgui_emulate_midi_keyboard();
    });

    if (tips_manager)
        DebugOptions::test_tips([&]() {
            test_tips(*tips_manager);
        });

#if defined(DEBUG)
    DebugOptions::color_themes_advanced_config_window([]() {
        color_themes()->imgui_advanced_config();
    });
#endif

#if defined(DEBUG)
    DebugOptions::show_all_icons([]() {
        imgui_show_all_icons();
    });
#endif

    DebugOptions::color_themes_editor([]() {
        color_themes()->imgui_basic_theme_editor();
    });
}

} // namespace Cool