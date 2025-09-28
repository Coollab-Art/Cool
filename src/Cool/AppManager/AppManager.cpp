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
#include "Cool/ImGui/apply_imgui_style_scale_ifn.hpp"
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

namespace Cool {

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

AppManager::AppManager(ViewsManager& views, IApp& app, AppManagerConfig config)
    : _views{views}
    , _app{app}
    , _config{config}
{
    fix_tdr_delay::set_minimum_delay(60); // Fixes a GPU crash on Windows. See the documentation of the library: https://github.com/CoolLibs/fix-tdr-delay

    // Set callbacks
    // clang-format off
    glfwSetWindowUserPointer         (window().glfw(), reinterpret_cast<void*>(this));
    glfwSetKeyCallback               (window().glfw(), AppManager::key_callback);
    glfwSetMouseButtonCallback       (window().glfw(), ImGui_ImplGlfw_MouseButtonCallback);
    glfwSetScrollCallback            (window().glfw(), ImGui_ImplGlfw_ScrollCallback);
    glfwSetCursorPosCallback         (window().glfw(), ImGui_ImplGlfw_CursorPosCallback);
    glfwSetCharCallback              (window().glfw(), ImGui_ImplGlfw_CharCallback);
    glfwSetWindowFocusCallback       (window().glfw(), ImGui_ImplGlfw_WindowFocusCallback);
    glfwSetCursorEnterCallback       (window().glfw(), ImGui_ImplGlfw_CursorEnterCallback);
    glfwSetWindowContentScaleCallback(_window_manager.main_window().glfw(), window_content_scale_callback);
    glfwSetMonitorCallback           (                 ImGui_ImplGlfw_MonitorCallback);
    // clang-format on
    register_initial_window_content_scale(_window_manager.main_window().glfw());
    ffmpeg::set_fast_seeking_callback([&]() { request_rerender_thread_safe(); });
    ffmpeg::set_frame_decoding_error_callback([&](std::string const& error_message) { Log::internal_warning("Video", error_message); });
    wcam::set_image_type<WebcamImage>();
}

auto AppManager::should_close_window() const -> bool
{
    if (!glfwWindowShouldClose(window().glfw()))
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

    glfwSetWindowShouldClose(window().glfw(), false);
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
    auto update_thread = std::jthread
    {
        [&]() {
            NFD::Guard nfd_guard{};
            while (!should_close_window())
            {
                do_update();
                should_stop = true;
            }
        };
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
            || ImGui::IsKeyChordPressed(ImGuiMod_Shortcut | ImGuiMod_Shift | ImGuiKey_I))
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

    static auto imgui_viewport_size() -> img::Size
    {
        auto const size = ImGui::GetIO().DisplayFramebufferScale * ImGui::GetIO().DisplaySize;
        return {
            static_cast<uint32_t>(size.x),
            static_cast<uint32_t>(size.y),
        };
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
        webgpu_context().encoder                  = webgpu_context().device.createCommandEncoder(wgpu::Default);
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
        if (texture_library_image().update()) // update() needs to be called because update has side effect
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
        apply_imgui_style_scale_ifn(); // Must be done before imgui_new_frame()

        imgui_new_frame();
        webgpu_context().check_for_swapchain_rebuild(imgui_viewport_size()); // Must be called after imgui_new_frame() because it sets up the imgui viewport size for the current frame // We can't use glfw's framebuffer size because when resizing the window it gets desynced for one frame with ImGui's viewport size (it only happens because we update the app on a separate thread, not on the main thread) // We check each frame instead of doing it in the glfw window resize callback, because we update on a separate thread, and events are handled on the main thread, which can cause data races and crashes.
        check_for_imgui_item_picker_request();
        imgui_render(_app);
        bool const can_present = imgui_end_frame();
        dispatch_all_events(); // Must be after `imgui_render()` in order for the extra_widgets on the Views to tell us wether we are allowed to dispatch View events.
        for (auto& view : _views)
            view->on_frame_end();
        TextureLibrary_FromWebcam::instance().on_frame_end();

        wgpu::CommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label   = "Cool Global Command Buffer";
        wgpu::CommandBuffer command = webgpu_context().encoder.finish(cmdBufferDescriptor);
        webgpu_context().encoder.release();
        webgpu_context().queue.submit(command);
        command.release();
#ifndef __EMSCRIPTEN__
        if (can_present)
            webgpu_context().surface.present();
#endif

// Check for pending error callbacks, and handle async callbacks
#ifdef WEBGPU_BACKEND_WGPU
        webgpu_context().queue.submit(0, nullptr);
#else
    webgpu_context().device.tick();
#endif
    }

    static void imgui_new_frame()
    {
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        imgui_dockspace();
    }

    void AppManager::imgui_render(IApp & app)
    {
        float window_title_height_bias = 0.f;

        // Apply normal font. The default font is the bold one because the window titles can only use the default font. We then have to push the regular() font back.
        window_title_height_bias += ImGui::GetFontSize();
        ImGui::PushFont(Font::regular());
        window_title_height_bias -= ImGui::GetFontSize();

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
        window_title_height_bias += 2.f * ImGui::GetStyle().FramePadding.y;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImGuiExtras::GetStyle().frame_padding);
        window_title_height_bias -= 2.f * ImGui::GetStyle().FramePadding.y;

        ImGui::GetCurrentContext()->window_title_height_bias = window_title_height_bias;

        // Windows
        app.imgui_windows();
        imgui_windows();
        ImGuiNotify::render_windows();

        ImGui::PopStyleVar();
        ImGui::PopFont();
        ImGui::GetStyle().FramePadding = ImGuiExtras::GetStyle().tab_bar_padding; // We need to apply the tab_bar_padding here because simply changing it when the style editor UI changes it doesn't work because it is in the middle of the ImGui::PushStyleVar(ImGuiStyleVar_FramePadding) that we do above.
    }

    auto AppManager::imgui_end_frame() -> bool
    {
        ImGui::EndFrame();
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (!draw_data)
            return false;
        {
            // HACK(ImNodes) Since nodes' workspace is not rendered during the first frame,
            // to avoid a flash we skip ImGui display altogether for the first few frames.
            // NB: go check out the other "HACK(ImNodes)"
            _frames_count++;
            if (_frames_count <= 2)
                return false;
        }
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        wgpu::SurfaceTexture surface_texture;
        webgpu_context().surface.getCurrentTexture(&surface_texture);
        if (surface_texture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) // TODO(WebGPU) Handle the various failure cases
        {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl; // TODO(WebGPU) Can this legitimately happen, ot is this always an error we need to handle / report

            return false; // TODO(WebGPU) We still want to update, but not render
        }
        auto view_descriptor                  = wgpu::TextureViewDescriptor{};
        view_descriptor.label                 = "Surface texture view";
        view_descriptor.format                = wgpuTextureGetFormat(surface_texture.texture);
        view_descriptor.dimension             = WGPUTextureViewDimension_2D;
        view_descriptor.baseMipLevel          = 0;
        view_descriptor.mipLevelCount         = 1;
        view_descriptor.baseArrayLayer        = 0;
        view_descriptor.arrayLayerCount       = 1;
        view_descriptor.aspect                = wgpu::TextureAspect::All;
        wgpu::TextureView          targetView = wgpuTextureCreateView(surface_texture.texture, &view_descriptor);
        wgpu::RenderPassDescriptor renderPassDesc{};

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};
        renderPassColorAttachment.view          = targetView;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp        = wgpu::LoadOp::Clear;
        renderPassColorAttachment.storeOp       = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue    = wgpu::Color{0., 0., 0., 1.};
        renderPassDesc.colorAttachmentCount     = 1;
        renderPassDesc.colorAttachments         = &renderPassColorAttachment;

        renderPassDesc.label           = "ImGui";
        renderPassDesc.timestampWrites = nullptr;
        webgpu_context().encoder.pushDebugGroup("ImGui");
        auto render_pass = webgpu_context().encoder.beginRenderPass(renderPassDesc);
        ImGui_ImplWGPU_RenderDrawData(draw_data, render_pass);
        render_pass.end();
        webgpu_context().encoder.popDebugGroup();
        render_pass.release();
        targetView.release();

        return true;
    }

    static void imgui_dockspace()
    {
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) // NOLINT
        {
            constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
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

    void AppManager::request_rerender_thread_safe()
    {
        _wants_to_request_rerender.store(true);
    }

    static AppManager& get_app_manager(GLFWwindow * glfw_window)
    {
        return *reinterpret_cast<AppManager*>(glfwGetWindowUserPointer(glfw_window)); // NOLINT
    }

    void AppManager::key_callback(GLFWwindow * glfw_window, int key, int scancode, int action, int mods)
    {
        auto& app_manager = get_app_manager(glfw_window);
        if (app_manager._config.dispatch_keyboard_events_to_imgui
            || ImGui::GetIO().WantTextInput) // We still want to allow shortcuts while in text input, like CTRL + SUPPR
        {
            ImGui_ImplGlfw_KeyCallback(glfw_window, key, scancode, action, mods);
        }
        window().check_for_fullscreen_toggles(key, scancode, action, mods);
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
