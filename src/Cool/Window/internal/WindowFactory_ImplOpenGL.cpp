#include <stdexcept>
#include "Cool/CommandLineArgs/CommandLineArgs.h"
#include "GLFW/glfw3.h"
#if defined(COOL_OPENGL)
#include "WindowFactory_ImplOpenGL.h" // Must be included first
//
#include <Cool/Utils/Version.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include "GLDebugCallback.h"
#include "WindowFactoryU.h"

namespace Cool {

static void shutdown_imgui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void WindowFactory_ImplOpenGL::shut_down(WindowManager& window_manager)
{
    shutdown_imgui();
    ImGui::DestroyContext();
    window_manager.windows().clear();
    glfwTerminate();
#if DEBUG
    CoolGlDebug::shut_down();
#endif
}

#if DEBUG
static void setup_opengl_debugging()
{
    if constexpr (COOL_OPENGL_VERSION < 430)
        return;

    int flags; // NOLINT
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (!(flags & GL_CONTEXT_FLAG_DEBUG_BIT))
    {
        Log::internal_warning("WindowFactory_ImplOpenGL::setupGLDebugging", "OpenGL Debugging not supported");
        return;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}
#endif

static void setup_imgui(Window& window)
{
    static_assert(COOL_OPENGL_VERSION >= 330 && "ImGui requires at least OpenGL 3.3");
    ImGui_ImplGlfw_InitForOpenGL(window.glfw(), false);
    std::string const glsl_version = fmt::format("#version {}", COOL_OPENGL_VERSION);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());
}

void WindowFactory_ImplOpenGL::setup_main_window(Window& window)
{
#if DEBUG
    setup_opengl_debugging();
#endif
    setup_imgui(window);
}

void WindowFactory_ImplOpenGL::setup_secondary_window(Window&, WindowManager& window_manager)
{
    glfwMakeContextCurrent(window_manager.main_window().glfw());
}

auto WindowFactory_ImplOpenGL::make_window(WindowConfig const& config, WindowManager& window_manager) -> Window&
{
    // Window flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version(COOL_OPENGL_VERSION));
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version(COOL_OPENGL_VERSION));
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#if DEBUG
    if constexpr (COOL_OPENGL_VERSION >= 430)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
    if (std::find(Cool::command_line_args().get().begin(), Cool::command_line_args().get().end(), "--hide_window") != Cool::command_line_args().get().end())
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // Create window
    auto&       windows      = window_manager.windows();
    GLFWwindow* other_window = windows.empty() ? nullptr : windows.back().glfw();
    auto&       window       = WindowFactoryU::make_window_with_glfw(config, window_manager, other_window);
    glfwMakeContextCurrent(window.glfw());
    WindowFactoryU::apply_config(config, window);
    // Load Glad
    if (!gladLoadGL(glfwGetProcAddress))
        throw std::runtime_error{"Failed to load OpenGL functions"};
    //
    return window;
}

} // namespace Cool

#endif