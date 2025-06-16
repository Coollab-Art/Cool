#include "AppManager.h"
#include "Audio/Audio.hpp"
#include "Cool/CommandLineArgs/CommandLineArgs.h"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/DebugOptions/frame_time_stopwatch.hpp"
#include "Cool/Gpu/Vulkan/Context.h"
#include "Cool/ImGui/ColorThemes.h"
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/ImGuiExtrasStyle.h"
#include "Cool/ImGui/StyleEditor.h"
#include "Cool/ImGui/apply_imgui_style_scale.hpp"
#include "Cool/ImGui/need_to_apply_imgui_style_scale.hpp"
#include "Cool/Input/MouseButtonEvent.h"
#include "Cool/Input/MouseCoordinates.h"
#include "Cool/Log/boxer_show.hpp"
#include "Cool/Midi/MidiManager.h"
#include "Cool/Task/TaskManager.hpp"
#include "Cool/TextureSource/TextureLibrary_Image.h"
#include "Cool/TextureSource/TextureLibrary_Video.h"
#include "Cool/TextureSource/TextureLibrary_Webcam.hpp"
#include "Cool/UI Scale/need_to_rebuild_fonts.hpp"
#include "Cool/UI Scale/screen_dpi_scale.hpp"
#include "Cool/UserSettings/UserSettings.h"
#include "Cool/Webcam/WebcamImage.hpp"
#include "Cool/Window/internal/imgui_build_fonts_ifn.hpp"
#include "GLFW/glfw3.h"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "easy_ffmpeg/callbacks.hpp"
#include "fix_tdr_delay/fix_tdr_delay.hpp"
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/imgui_internal.h"
#include "nfd.hpp"
#include "wcam/wcam.hpp"

#if defined(COOL_VULKAN)
#include <imgui/backends/imgui_impl_vulkan.h>
#elif defined(COOL_OPENGL)
#include <imgui/backends/imgui_impl_opengl3.h>
#endif

namespace Cool {

static void prepare_windows(WindowManager& window_manager);
static void imgui_dockspace();
static void imgui_new_frame();

static void window_content_scale_callback(GLFWwindow* /* window */, float x_scale, float y_scale)
{
#if defined(__APPLE__)
    // HACK: on MacOS we detect a scale of 2 for retina screens, but it seems like ImGui already takes it into account because if we apply that scale of 2 ourselves the UI appears too big by a factor of 2.
    x_scale = 1.f;
    y_scale = 1.f;
#endif
    if (Cool::DebugOptions::log_ui_scale_changes())
        Cool::Log::info("UI Scale", fmt::format("DPI Scale: {} x {}", x_scale, y_scale));

    assert(x_scale == y_scale);
    screen_dpi_scale()                = y_scale;
    need_to_rebuild_fonts()           = true;
    need_to_apply_imgui_style_scale() = true;
}

static void register_initial_window_content_scale(GLFWwindow* window)
{
    float x_scale{1.f};
    float y_scale{1.f};
    glfwGetWindowContentScale(window, &x_scale, &y_scale);
    window_content_scale_callback(window, x_scale, y_scale);
}

AppManager::AppManager(WindowManager& window_manager, ViewsManager& views, IApp& app, AppManagerConfig config)
    : _window_manager{window_manager}
    , _views{views}
    , _app{app}
    , _config{config}
{
    fix_tdr_delay::set_minimum_delay(60); // Fixes a GPU crash on Windows. See the documentation of the library: https://github.com/Coollab-Art/fix-tdr-delay
    // Set callbacks
    for (auto& window : _window_manager.windows())
    {
        GLFWwindow* glfw_window = window.glfw();
        glfwSetWindowUserPointer(glfw_window, reinterpret_cast<void*>(this)); // NOLINT
        if (glfw_window != _window_manager.main_window().glfw())
        {
            glfwSetKeyCallback(glfw_window, AppManager::key_callback_for_secondary_windows);
            glfwSetWindowCloseCallback(glfw_window, AppManager::window_close_callback_for_secondary_windows);
        }
    }
    // clang-format off
    glfwSetKeyCallback               (_window_manager.main_window().glfw(), AppManager::key_callback);
    glfwSetMouseButtonCallback       (_window_manager.main_window().glfw(), ImGui_ImplGlfw_MouseButtonCallback);
    glfwSetScrollCallback            (_window_manager.main_window().glfw(), ImGui_ImplGlfw_ScrollCallback);
    glfwSetCursorPosCallback         (_window_manager.main_window().glfw(), ImGui_ImplGlfw_CursorPosCallback);
    glfwSetCharCallback              (_window_manager.main_window().glfw(), ImGui_ImplGlfw_CharCallback);
    glfwSetWindowFocusCallback       (_window_manager.main_window().glfw(), ImGui_ImplGlfw_WindowFocusCallback);
    glfwSetCursorEnterCallback       (_window_manager.main_window().glfw(), ImGui_ImplGlfw_CursorEnterCallback);
    glfwSetWindowContentScaleCallback(_window_manager.main_window().glfw(), window_content_scale_callback);
    glfwSetMonitorCallback           (                                      ImGui_ImplGlfw_MonitorCallback);
    // clang-format on
    register_initial_window_content_scale(_window_manager.main_window().glfw());
    ffmpeg::set_fast_seeking_callback([&]() { request_rerender_thread_safe(); });
    ffmpeg::set_frame_decoding_error_callback([&](std::string const& error_message) { Log::internal_warning("Video", error_message); });
    wcam::set_image_type<WebcamImage>();
}

auto AppManager::should_close_window() const -> bool
{
    if (!glfwWindowShouldClose(_window_manager.main_window().glfw()))
        return false;

    auto const tasks_in_progress = task_manager().list_of_tasks_that_need_user_confirmation_before_killing();
    if (tasks_in_progress.empty())
        return true;

    auto const choice = boxer_show(
        ("There are some tasks in progress, if you exit now they will not be able to complete successfully:\n" + tasks_in_progress).c_str(),
        "Kill tasks in progress?",
        boxer::Style::Warning,
        boxer::Buttons::OKCancel
    );
    if (choice == boxer::Selection::OK)
        return true;

    glfwSetWindowShouldClose(_window_manager.main_window().glfw(), false);
    return false;
}

void AppManager::close_application()
{
    _window_manager.main_window().close();
}

void AppManager::close_application_if_all_tasks_are_done()
{
    if (!task_manager().list_of_tasks_that_need_user_confirmation_before_killing().empty())
        return;
    close_application();
}

void AppManager::run(std::function<void()> const& on_update)
{
    _app.init();
    auto const do_update = [&]() {
        try
        {
            update();
            on_update();
            wcam::update();
        }
        catch (std::exception const& e)
        {
            Log::internal_error("UNKNOWN ERROR 1", e.what());
        }
    };
#if defined(COOL_UPDATE_APP_ON_SEPARATE_THREAD)
    auto should_stop   = false;
    auto update_thread = std::jthread{[&]() {
        NFD::Guard nfd_guard{};
        while (!should_close_window())
        {
            do_update();
        }
        should_stop = true;
    }};
    while (!should_stop)
    {
        glfwWaitEvents();
    }
#else
    NFD::Guard nfd_guard{};
    while (!should_close_window())
    {
        if (_frames_count > 1) // Skip first frame which is very slow and messes up the initial plot
        {
            frame_time_stopwatch().stop();
            frame_time_stopwatch().start();
        }
        glfwPollEvents();
        do_update();
    }
#endif
    // Restore any ImGui ini state that might have been stored
    _app._wants_to_restore_ini_state = true;
    restore_imgui_ini_state_ifn();
    // Make sure no tasks are running, as they might need things to still be alive in order to finish their job
    task_manager().shut_down();
}

static void check_for_imgui_item_picker_request()
{
#if DEBUG
    if (DebugOptions::imgui_item_picker()
        || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_I))
    {
        ImGui::DebugStartItemPicker();
    }
#endif
}

void AppManager::restore_imgui_ini_state_ifn()
{
    if (!_app._wants_to_restore_ini_state
        || !_app._imgui_ini_state_to_restore.has_value())
        return;

    ImGui::LoadIniSettingsFromMemory(_app._imgui_ini_state_to_restore->c_str());
    _app._wants_to_restore_ini_state = false;
    _app._imgui_ini_state_to_restore.reset();
}

void AppManager::update()
{
    color_themes()->update();
    // Cache these colors for the frame, because we don't want to query the Theme all the time.
    // They will be reused by a few things event outside of ImGui::Notify
    ImGuiNotify::get_style().color_success          = color_themes()->editor().get_color("Success").as_imvec4();
    ImGuiNotify::get_style().color_warning          = color_themes()->editor().get_color("Warning").as_imvec4();
    ImGuiNotify::get_style().color_error            = color_themes()->editor().get_color("Error").as_imvec4();
    ImGuiNotify::get_style().color_info             = color_themes()->editor().get_color("Accent").as_imvec4();
    ImGuiNotify::get_style().color_title_background = ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg);

    user_settings().update();

    ImGui::GetIO().ConfigDragClickToInputText = user_settings().single_click_to_input_in_drag_widgets;
    prepare_windows(_window_manager);
#if defined(COOL_VULKAN)
    vkDeviceWaitIdle(Vulkan::context().g_Device);
#endif
    if (DebugOptions::show_command_line_arguments())
    {
        for (auto const& arg : command_line_args().get())
            Log::info("cli", arg);
    }
    midi_manager().check_for_devices();
    Audio::player().update_device_if_necessary();
    if (TextureLibrary_Image::instance().update()) // update() needs to be called because update has side effect
        _app.request_rerender();
    TextureLibrary_Video::instance().update();
    if (_wants_to_request_rerender.load())
    {
        _app.request_rerender();
        _wants_to_request_rerender.store(false);
    }
    try
    {
        _app.update();
    }
    catch (std::exception const& e)
    {
        Log::internal_error("UNKNOWN ERROR 2", e.what());
    }
    task_manager().update_on_main_thread();

    restore_imgui_ini_state_ifn(); // Must be done before imgui_new_frame() (this is a constraint from Dear ImGui (https://github.com/ocornut/imgui/issues/6263#issuecomment-1479727227))
    imgui_build_fonts_ifn();       // Must be done before imgui_new_frame()
    apply_imgui_style_scale_ifn();

    imgui_new_frame();
    check_for_imgui_item_picker_request();
    imgui_render(_app);
    dispatch_all_events(); // Must be after `imgui_render()` in order for the extra_widgets on the Views to tell us wether we are allowed to dispatch View events.
    for (auto& view : _views)
        view->on_frame_end();
    TextureLibrary_Webcam::instance().on_frame_end();
    end_frame(_window_manager);
}

static void prepare_windows(WindowManager& window_manager)
{
#if defined(COOL_VULKAN)
    for (auto& window : window_manager.windows())
    {
        window.check_for_swapchain_rebuild();
    }
#elif defined(COOL_OPENGL)
    glfwMakeContextCurrent(window_manager.main_window().glfw());
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
}

static void imgui_new_frame()
{
#if defined(COOL_VULKAN)
    ImGui_ImplVulkan_NewFrame();
#elif defined(COOL_OPENGL)
    ImGui_ImplOpenGL3_NewFrame();
#endif
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    imgui_dockspace();
}

void AppManager::imgui_render(IApp& app)
{
    float title_bar_height_bias = 0.f;

    // Apply normal font. The default font is the bold one because the window titles can only use the default font. We then have to push the regular() font back.
    title_bar_height_bias += ImGui::GetFontSize();
    ImGui::PushFont(Font::regular());
    title_bar_height_bias -= ImGui::GetFontSize();

    // Menu bar
    if (app.wants_to_show_menu_bar())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImGuiExtras::GetStyle().menu_bar_spacing);
        ImGui::BeginMainMenuBar();
        app.imgui_menus();
        ImGui::EndMainMenuBar();
        ImGui::PopStyleVar();
    }

    // Apply normal FramePadding. The one stored in ImGui::GetStyle() is used by the window titles only.
    title_bar_height_bias += 2.f * ImGui::GetStyle().FramePadding.y;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImGuiExtras::GetStyle().frame_padding);
    title_bar_height_bias -= 2.f * ImGui::GetStyle().FramePadding.y;

    ImGui::GetCurrentContext()->title_bar_height_bias = title_bar_height_bias;

    // Windows
    app.imgui_windows();
    imgui_windows();
    ImGuiNotify::render_windows();

    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::GetStyle().FramePadding = ImGuiExtras::GetStyle().tab_bar_padding; // We need to apply the tab_bar_padding here because simply changing it when the style editor UI changes it doesn't work because it is in the middle of the ImGui::PushStyleVar(ImGuiStyleVar_FramePadding) that we do above.
}

void AppManager::end_frame(WindowManager& window_manager)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    {
        // HACK(ImNodes) Since nodes' workspace is not rendered during the first frame,
        // to avoid a flash we skip ImGui display altogether for the first few frames.
        // NB: go check out the other "HACK(ImNodes)"
        _frames_count++;
        if (_frames_count <= 2)
            draw_data = nullptr;
    }
#if defined(COOL_VULKAN)
    const bool can_render = draw_data && draw_data->DisplaySize.x > 0.0f && draw_data->DisplaySize.y > 0.0f;
    if (can_render)
    {
        window_manager.main_window().FrameRender(draw_data);
    }
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    if (can_render)
    {
        window_manager.main_window().FramePresent();
    }
#elif defined(COOL_OPENGL)
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
    if (draw_data)
        ImGui_ImplOpenGL3_RenderDrawData(draw_data);
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) // NOLINT
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
    glfwSwapBuffers(window_manager.main_window().glfw());
#endif
}

static void imgui_dockspace()
{
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) // NOLINT
    {
        constexpr ImGuiDockNodeFlags dockspace_flags = static_cast<int>(ImGuiDockNodeFlags_PassthruCentralNode) | static_cast<int>(ImGuiDockNodeFlags_NoCloseButton);
        constexpr ImGuiWindowFlags   window_flags    = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("MyMainDockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        ImGui::End();
    }
}

static AppManager& get_app_manager(GLFWwindow* window)
{
    return *reinterpret_cast<AppManager*>(glfwGetWindowUserPointer(window)); // NOLINT
}

void AppManager::request_rerender_thread_safe()
{
    _wants_to_request_rerender.store(true);
}

void AppManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto& app_manager = get_app_manager(window);
    if (app_manager._config.dispatch_keyboard_events_to_imgui
        || ImGui::GetIO().WantTextInput) // We still want to allow shortcuts while in text input, like CTRL + SUPPR
    {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
    app_manager._window_manager.main_window().check_for_fullscreen_toggles(key, scancode, action, mods);
}

void AppManager::key_callback_for_secondary_windows(GLFWwindow* glfw_window, int key, int scancode, int action, int mods)
{
    get_app_manager(glfw_window)
        ._window_manager.find(glfw_window)
        .check_for_fullscreen_toggles(key, scancode, action, mods);
}

void AppManager::window_close_callback_for_secondary_windows(GLFWwindow* glfw_window)
{
    Window& window = get_app_manager(glfw_window)._window_manager.find(glfw_window);
    if (window.is_visible() && glfwWindowShouldClose(window.glfw()))
    {
        window.set_visibility(false);
        glfwSetWindowShouldClose(window.glfw(), GLFW_FALSE);
    }
}

void AppManager::dispatch_all_events()
{
    if (!_app.inputs_are_allowed())
        return;
    dispatch_mouse_movement();
    dispatch_mouse_click();
    dispatch_mouse_scroll();
}

void AppManager::dispatch_mouse_movement()
{
    if (ImGui::GetIO().MouseDelta == ImVec2{0.f, 0.f})
        return;

    auto const event = MouseMoveEvent<ImGuiCoordinates>{
        .position = ImGui::GetIO().MousePos,
        .delta    = ImGui::GetIO().MouseDelta,
    };
    for (auto& view : _views)
        view->dispatch_mouse_move_event(event);
}

void AppManager::dispatch_mouse_click()
{
    for (int button = 0; button < IM_ARRAYSIZE(ImGui::GetIO().MouseClicked); button++)
    {
        if (ImGui::IsMouseClicked(button))
        {
            auto const event = MouseButtonEvent<ImGuiCoordinates>{
                .position = ImGui::GetIO().MousePos,
                .button   = button,
                .action   = ButtonAction::Pressed,
            };
            for (auto& view : _views)
                view->dispatch_mouse_button_event(event);
        }
        if (ImGui::IsMouseReleased(button))
        {
            auto const event = MouseButtonEvent<ImGuiCoordinates>{
                .position = ImGui::GetIO().MousePos,
                .button   = button,
                .action   = ButtonAction::Released,
            };
            for (auto& view : _views)
                view->dispatch_mouse_button_event(event);
        }
    }
}

void AppManager::dispatch_mouse_scroll()
{
    float const scroll_x = ImGui::GetIO().MouseWheelH;
    float const scroll_y = ImGui::GetIO().MouseWheel;
    if (scroll_x == 0.f && scroll_y == 0.f)
        return;

    auto const event = MouseScrollEvent<ImGuiCoordinates>{
        .position = ImGui::GetIO().MousePos,
        .dx       = scroll_x,
        .dy       = scroll_y,
    };

    for (auto& view : _views)
        view->dispatch_mouse_scroll_event(event);
}

void AppManager::imgui_windows()
{
    Cool::DebugOptions::style_editor([&]() {
        style_editor()->imgui();
    });
}

} // namespace Cool
