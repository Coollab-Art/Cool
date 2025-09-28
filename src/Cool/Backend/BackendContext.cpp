#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>
//
#include <Cool/AppManager/should_we_use_a_separate_thread_for_update.h>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_wgpu.h>
#include <imgui/imgui.h>
#include <stdexcept>
#include "BackendContext.h"
#include "Cool/Backend/WindowConfig.h"
#include "Cool/Backend/imgui_config.h"
#include "Cool/Log/ToUser.h"
#include "Cool/Path/Path.h"
#include "Cool/WebGPU/WebGPUContext.hpp"

namespace Cool {

static void glfw_error_callback(int error, const char* description)
{
    Log::internal_error("glfw", fmt::format("Error {}: {}", error, description));
}

#if DEBUG
static void setup_webgpu_debugging()
{
    // if constexpr (COOL_OPENGL_VERSION < 430)
    //     return;

    // int flags; // NOLINT
    // glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    // if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    // {
    //     glEnable(GL_DEBUG_OUTPUT);
    //     glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    //     glDebugMessageCallback(GLDebugCallback, nullptr);
    //     glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    // }
    // else
    // {
    //     Log::Debug::warning(
    //         "WindowFactory_ImplOpenGL::setupGLDebugging",
    //         "Couldn't setup OpenGL Debugging"
    //     );
    // }
}
#endif

static void throw_error(std::string const& error_message)
{
    // TODO(WebGPU) this todo below should be done, but make sure the log file is already created so we can properly log. Also, all the Log:: functions should log to cout / cerr in debug mode to help us debugging?
    // and should we really do Log::internal_error ? If yes, do it with two parts "message" and "err" like in the commented code
    // NB: i had removed the logging on main branch, because all exceptions are already logged
    // TODO(WebGPU) Log to a file too, so that we can debug when the app is shipped to end customers who don't have a debugger attached
    // #if DEBUG
    //     std::cerr << error_message << '\n';
    // #endif
    // Log::internal_error(message, err);
    // throw std::runtime_error{fmt::format("{}:\n{}", message, err)};
    throw std::runtime_error{error_message};
}

static void throw_glfw_error(std::string_view message)
{
    const char* err; // NOLINT(*-init-variables)
    glfwGetError(&err);
    throw_error(fmt::format("{}:\n{}", message, err));
}

static void set_window_icon(GLFWwindow* window)
{
#if defined(COOL_APP_ICON_FILE)                                                                // Don't do anything if no icon has been set
    auto icon  = *img::load(Cool::Path::root() / COOL_APP_ICON_FILE, 4, img::FirstRowIs::Top); // TODO(WebGPU) default icon if we fail to find it? instead of crashing. Or just do nothing?
    auto image = GLFWimage{
        .width  = static_cast<int>(icon.width()),
        .height = static_cast<int>(icon.height()),
        .pixels = icon.data(),
    };
    glfwSetWindowIcon(window, 1, &image);
#else
    std::ignore = window; // Prevent warning
#endif
}

void apply_config(WindowConfig const& config, Window& window)
{
    window.cap_framerate_if(config.cap_framerate_on_startup_if);
    if (config.maximize_on_startup_if)
    {
        glfwMaximizeWindow(window.glfw());
    }
    if (config.hide_on_startup_if)
    {
        window.set_visibility(false);
    }
}

// TODO(WebGPU) Disable validation layers in release

static auto to_string(wgpu::ErrorType type) -> const char*
{
    switch (type)
    {
    case WGPUErrorType_Validation: return "Validation";
    case WGPUErrorType_OutOfMemory: return "Out of memory";
    case WGPUErrorType_Unknown: return "Unknown";
    case WGPUErrorType_DeviceLost: return "Device lost";
    case WGPUErrorType_Internal: return "Internal";
    default:
    {
        assert(false);
        return "UNKNOWN";
    }
    }
}

static auto size_in_pixels(WindowSize size, int screen_size) -> int
{
    return std::visit(
        Cool::overloaded{
            [&](SizeProportionalToScreen sz) {
                return static_cast<int>(static_cast<float>(screen_size) * sz.proportion);
            },
            [&](SizeInPixels sz) {
                return sz.pixels;
            }
        },
        size
    );
}

BackendContext::BackendContext(WindowConfig const& config)
{
    // WebGPU
    _wgpu.instance = wgpu::createInstance(wgpu::InstanceDescriptor{});
    if (!webgpu_context().instance)
        throw_error("WebGPU initialization failed");

    // GLFW
    glfwSetErrorCallback(&glfw_error_callback);
    if (!glfwInit())
        throw_glfw_error("GLFW initialization failed");

    // GLFW window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    {
        GLFWvidmode const* const video_mode    = glfwGetVideoMode(glfwGetPrimaryMonitor());
        int const                window_width  = size_in_pixels(config.initial_width, video_mode->width);
        int const                window_height = size_in_pixels(config.initial_height, video_mode->height);

        _window = Window{glfwCreateWindow(window_width, window_height, config.title.c_str(), nullptr, nullptr)};
        if (!window().glfw())
            throw_glfw_error("Updating your graphics card drivers might solve the issue.\n\nWindow creation failed"); // TODO(WebGPU) check how this error looks (and tell about antivirus? only on windows/linux? and add a todo to tell to remove this once we sign exe on windows)

        // Center the window
        glfwSetWindowPos(_window.glfw(), (video_mode->width - window_width) / 2, (video_mode->height - window_height) / 2);
    }
    set_window_icon(window().glfw());
    apply_config(config, _window);

    // WebGPU
    _wgpu.surface = glfwGetWGPUSurface(_wgpu.instance, window().glfw());
    wgpu::RequestAdapterOptions adapterOpts{};
    adapterOpts.powerPreference   = wgpu::PowerPreference::HighPerformance;
    adapterOpts.compatibleSurface = _wgpu.surface;
    _wgpu.adapter                 = _wgpu.instance.requestAdapter(adapterOpts);
    _wgpu.surface_format          = _wgpu.surface.getPreferredFormat(_wgpu.adapter);

    wgpu::DeviceDescriptor deviceDesc;
    deviceDesc.requiredFeatureCount = 0;
    _wgpu.device                    = _wgpu.adapter.requestDevice(deviceDesc);

    // Add an error callback for more debug info
#if DEBUG
    _wgpu_error_callback = _wgpu.device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
        throw_error(fmt::format("WebGPU {} error:\n{}", to_string(type), message)); // TODO(WebGPU) Don't throw on all errors?
    });
    // TODO(WebGPU)
    // _wgpu.device.setDeviceLostCallback(DeviceLostCallback &&callback)
#endif

    _wgpu.queue = _wgpu.device.getQueue();

    {
        // Create swapchain for the first time
        int width, height; // NOLINT(*isolate-declaration, *init-variables)
        glfwGetFramebufferSize(window().glfw(), &width, &height);
        _wgpu.check_for_swapchain_rebuild({static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOther(window().glfw(), /*install_callbacks=*/false);
    ImGui_ImplWGPU_Init(webgpu_context().device, 3, webgpu_context().surface_format);
    imgui_config();
}

BackendContext::~BackendContext()
{
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window().glfw());
    glfwTerminate();
}

} // namespace Cool