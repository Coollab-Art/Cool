# On top of the usual `target_link_libraries`, you must also call `cool_setup` with your target name. E.g.:
# ```
# add_subdirectory(Cool)
# target_link_libraries(${PROJECT_NAME} PRIVATE Cool::Core)
# cool_setup(${PROJECT_NAME})
# ```

cmake_minimum_required(VERSION 3.20)

option(COOL_USE_MACOS_BUNDLE "Turn this on if you are using an App Bundle on MacOS (this option does nothing if you are not building for MacOS)" OFF)

add_library(Cool)
add_library(Cool::Core ALIAS Cool)
add_library(Cool-PublicProperties INTERFACE) # This is needed by both Cool and its tests
add_library(Cool-PrivateProperties INTERFACE) # This is needed by both Cool and its tests
target_link_libraries(Cool PUBLIC Cool-PublicProperties)
target_link_libraries(Cool PRIVATE Cool-PrivateProperties)

# Choose C++ version
target_compile_features(Cool-PublicProperties INTERFACE cxx_std_20)

if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()

# Set warning level
if(MSVC)
    target_compile_options(Cool-PrivateProperties INTERFACE /W4)
else()
    target_compile_options(Cool-PrivateProperties INTERFACE -Wall -Wextra -Wpedantic -pedantic-errors -Wconversion -Wsign-conversion -Wimplicit-fallthrough)
endif()

# Maybe enable warnings as errors
set(WARNINGS_AS_ERRORS_FOR_COOL OFF CACHE BOOL "ON iff you want to treat warnings as errors") # Might be overriden in the CMake cache

if(WARNINGS_AS_ERRORS_FOR_COOL)
    if(MSVC)
        target_compile_options(Cool-PrivateProperties INTERFACE /WX)
    else()
        target_compile_options(Cool-PrivateProperties INTERFACE -Werror)
    endif()
endif()

# Set COOL_PATH as the path to Cool, relative to the main project's CMakeLists.txt. It will look like "third-party/Cool"
cmake_path(RELATIVE_PATH CMAKE_CURRENT_SOURCE_DIR BASE_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE COOL_PATH)
set(COOL_PATH_INTERNAL ${COOL_PATH} CACHE INTERNAL "") # Make a global variable so that `cool_setup()` will be able to use it when it is called outside of this CMake file

target_compile_definitions(Cool-PublicProperties INTERFACE
    $<$<CONFIG:Debug>:DEBUG> # Define DEBUG in debug mode
    COOL_PATH=\"${COOL_PATH}\" # This is useful to set the path to Cool's res directory
    $<$<CONFIG:Debug>:COOL_DEBUG_ONLY__CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"> # This is useful to set the current working directory in debug mode.
    IMGUI_DEFINE_MATH_OPERATORS
    COOL_APP_NAME=\"${COOL_APP_NAME}\"
)

if(COOL_USE_MACOS_BUNDLE)
    target_compile_definitions(Cool-PrivateProperties INTERFACE COOL_USE_MACOS_BUNDLE=1)
else()
    target_compile_definitions(Cool-PrivateProperties INTERFACE COOL_USE_MACOS_BUNDLE=0)
endif()

# ----------------------
# ---Add Cool sources---
# ----------------------
file(GLOB_RECURSE COOL_SOURCES CONFIGURE_DEPENDS src/*)
set(COOL_APP_VERSION_STRING "\"${COOL_APP_VERSION}\"")
configure_file("src/Cool/Dump/app_version.cpp.in" "src/Cool/Dump/app_version.cpp")
list(APPEND COOL_SOURCES "src/Cool/Dump/app_version.cpp")
target_sources(Cool PRIVATE ${COOL_SOURCES})

# -----------------------------
# ---Set include directories---
# -----------------------------
target_include_directories(Cool-PublicProperties INTERFACE src)
target_include_directories(Cool-PublicProperties SYSTEM INTERFACE lib)
target_precompile_headers(Cool-PublicProperties INTERFACE # More info on precompiled headers: https://julesfouchy.github.io/Learn--Cpp-And-Dev-Practices/docs/lessons/precompiled-header
    <Cool/UDL/std.h> # Make std literals available everywhere. E.g. "Hello"s will create a std::string, not a const char*.
    <vector>
    <string>
    <memory>
    <functional>
    <variant>
    <optional>
    <algorithm>
    <unordered_map>
    <deque>
    <thread>
    <utility>
    <chrono>
    <list>
    <imgui/imgui.h>
    <imgui/misc/cpp/imgui_stdlib.h>
    <glm/glm.hpp>
    <glm/gtc/type_ptr.hpp>
    <ser20/access.hpp>
    <ser20/types/vector.hpp>
    <ser20/types/string.hpp>
    <ser20/types/variant.hpp>
    <boxer/boxer.h>
    <nfd.hpp>
    <IconFontCppHeaders/IconsFontAwesome6.h>
    <scope_guard/scope_guard.hpp>
    <fmt/format.h>
    <fmt/core.h>
    <fmt/std.h>
    <fmt/compile.h>
    <gsl/gsl>
    <tl/expected.hpp>
    <Cool/Log/Log.hpp>
    <Cool/Geometry/imvec_glm_conversions.h>
    <Cool/Serialization/filesystem.h>
    <Cool/Serialization/img.h>
    <Cool/Serialization/GlmSerialization.h> # must be included after <glm/glm.hpp> because it depends on it
    <Cool/ostream/glm.h> # must be included after <glm/glm.hpp> because it depends on it
    <Cool/ostream/vector.h> # must be included after <vector> because it depends on it
)

# -------------------
# ---Add libraries---
# -------------------

# GLFW (Must be before WebGPU, otherwise Dawn uses its own version of glfw)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)
target_link_libraries(Cool-PublicProperties INTERFACE glfw)
install(FILES "lib/glfw/LICENSE.md" DESTINATION "license/glfw")

# WebGPU (Must be after GLFW, otherwise Dawn uses its own version of glfw)
# NB : for now WGPU doesn't compile wgsl properly, it is not up-to-date with the official syntax.
set(WEBGPU_BACKEND "WGPU" CACHE STRING "" FORCE) # TODO(WebGPU) Use Dawn when creating Release, because it links statically, and WGPU when developping because if it faster and more lightweight

if(WEBGPU_BACKEND STREQUAL "DAWN")
    # Needed by Dawn
    set(TINT_BUILD_WGSL_READER ON CACHE BOOL "" FORCE)
    set(TINT_BUILD_SPV_WRITER ON CACHE BOOL "" FORCE)

    # Needed by ShaderTranspiler
    set(TINT_BUILD_WGSL_WRITER ON CACHE BOOL "" FORCE)
    set(TINT_BUILD_SPV_READER ON CACHE BOOL "" FORCE)

    set(TINT_BUILD_GLSL_WRITER OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_HLSL_WRITER OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_MSL_WRITER OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_IR OFF CACHE BOOL "" FORCE)
endif()

add_subdirectory(lib/WebGPU-distribution)
add_subdirectory(lib/glfw3webgpu)
target_link_libraries(Cool-PublicProperties INTERFACE webgpu glfw3webgpu)

# ShaderTranspiler
set(ST_ENABLE_TEST OFF CACHE BOOL "" FORCE)
set(ST_ENABLE_WGSL ON CACHE BOOL "" FORCE)
add_subdirectory(lib/ShaderTranspiler)
target_link_libraries(Cool-PublicProperties INTERFACE ShaderTranspiler)

# folder_watcher
add_subdirectory(lib/folder_watcher)
target_link_libraries(Cool-PublicProperties INTERFACE folder_watcher::folder_watcher)

# imgui_gradient
add_subdirectory(lib/imgui_gradient)
target_include_directories(imgui_gradient SYSTEM PRIVATE lib/imgui) # imgui_gradient needs acces to imgui
target_link_libraries(Cool-PublicProperties INTERFACE imgui_gradient::imgui_gradient)

# algorithms
add_subdirectory(lib/algorithms)
target_link_libraries(Cool-PublicProperties INTERFACE algorithms::algorithms)

# scope_guard
install(FILES "lib/scope_guard/LICENSE" DESTINATION "license/scope_guard")

# ser20
add_subdirectory(lib/ser20)
target_link_libraries(Cool-PublicProperties INTERFACE ser20::ser20)
install(FILES "lib/ser20/LICENSE" DESTINATION "license/ser20")

# smart
add_subdirectory(lib/smart)
target_link_libraries(Cool-PublicProperties INTERFACE smart::smart)

# IconFontCppHeaders
install(FILES "lib/IconFontCppHeaders/licence.txt" DESTINATION "license/IconFontCppHeaders")

# Boxer
add_subdirectory(lib/Boxer)
target_link_libraries(Cool-PublicProperties INTERFACE Boxer)
install(FILES "lib/Boxer/LICENSE" DESTINATION "license/Boxer")

# Native File Dialog
set(NFD_OVERRIDE_RECENT_WITH_DEFAULT ON CACHE BOOL "" FORCE)
add_subdirectory(lib/nativefiledialog-extended EXCLUDE_FROM_ALL)
target_link_libraries(Cool-PublicProperties INTERFACE nfd)
install(FILES "lib/nativefiledialog-extended/LICENSE" DESTINATION "license/nativefiledialog-extended")

# GSL
add_subdirectory(lib/GSL)
target_link_libraries(Cool-PublicProperties INTERFACE Microsoft.GSL::GSL)
install(FILES "lib/GSL/LICENSE" DESTINATION "license/GSL")

# spdlog
target_include_directories(Cool-PrivateProperties SYSTEM INTERFACE lib/spdlog/include)
install(FILES "lib/spdlog/LICENSE" DESTINATION "license/spdlog")

# img
add_subdirectory(lib/img)
target_link_libraries(Cool-PublicProperties INTERFACE img::img)

# easy_ffmpeg
add_subdirectory(lib/easy_ffmpeg)
target_link_libraries(Cool-PublicProperties INTERFACE easy_ffmpeg::easy_ffmpeg)

# Audio
add_subdirectory(lib/Audio)
target_link_libraries(Cool-PublicProperties INTERFACE Cool::Audio)

# exe_path
add_subdirectory(lib/exe_path)
target_link_libraries(Cool-PublicProperties INTERFACE exe_path::exe_path)

# fix_tdr_delay
add_subdirectory(lib/fix-tdr-delay)
target_link_libraries(Cool-PrivateProperties INTERFACE fix_tdr_delay::fix_tdr_delay)

# serv
add_subdirectory(lib/serv)
target_link_libraries(Cool-PublicProperties INTERFACE serv::serv)

# stringify
add_subdirectory(lib/stringify)
target_link_libraries(Cool-PublicProperties INTERFACE stringify::stringify)

# op
add_subdirectory(lib/op)
target_link_libraries(Cool-PublicProperties INTERFACE op::op)

# reg
add_subdirectory(lib/reg)
target_link_libraries(Cool-PublicProperties INTERFACE reg::reg)

# wafl
add_subdirectory(lib/wafl)
target_link_libraries(Cool-PublicProperties INTERFACE wafl::wafl)

# open_link
add_subdirectory(lib/open)
target_link_libraries(Cool-PublicProperties INTERFACE Cool::open)

# os_name
add_subdirectory(lib/os_name)
target_link_libraries(Cool-PublicProperties INTERFACE os_name::os_name)

# tl::expected
target_include_directories(Cool-PublicProperties SYSTEM INTERFACE lib/expected/include)

# nlohmann json
target_include_directories(Cool-PublicProperties SYSTEM INTERFACE lib/json/single_include)
install(FILES "lib/json/LICENSE.MIT" DESTINATION "license/nlohmann_json")

# Dear ImGui
add_subdirectory(lib/imgui)
target_include_directories(Cool-PublicProperties SYSTEM INTERFACE lib/imgui)
install(FILES "lib/imgui/LICENSE.txt" DESTINATION "license/imgui")

# wcam
set(WCAM_DONT_INCLUDE_FMT ON)
add_subdirectory(lib/wcam)
target_link_libraries(Cool-PublicProperties INTERFACE wcam::wcam)

# get_system_error
# add_subdirectory(lib/get_system_error) # Already done by wcam
target_link_libraries(Cool-PublicProperties INTERFACE Cool::get_system_error)

# RtMidi
add_subdirectory(lib/RtMidiWrapper)
target_link_libraries(Cool-PublicProperties INTERFACE RtMidiW::RtMidiW)

# oscpack
add_subdirectory(lib/oscpack)
target_link_libraries(Cool-PrivateProperties INTERFACE oscpack)

# glm
# TODO(WebGPU) We are including glm multiple times ?
add_subdirectory(lib/glm)
target_link_libraries(Cool-PublicProperties INTERFACE glm)
target_compile_definitions(Cool-PublicProperties INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)
install(FILES "lib/glm/copying.txt" DESTINATION "license/glm")

# GLSL-Color-Functions
install(FILES "res/shaders/GLSL-Color-Functions/LICENSE" DESTINATION "license/GLSL-Color-Functions")

# Build ImGui with glfw and WebGPU
target_link_libraries(ImGui PRIVATE glfw)
target_sources(ImGui PRIVATE lib/imgui/backends/imgui_impl_glfw.cpp)
target_link_libraries(ImGui PRIVATE webgpu)
target_sources(ImGui PRIVATE lib/imgui/backends/imgui_impl_wgpu.cpp)

if(WEBGPU_BACKEND STREQUAL "DAWN")
    target_compile_definitions(ImGui PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_DAWN)
elseif(WEBGPU_BACKEND STREQUAL "WGPU")
    target_compile_definitions(ImGui PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_WGPU)
else()
    message(FATAL_ERROR "Unknown WebGPU backend \"${WEBGPU_BACKEND}\"")
endif()

target_link_libraries(Cool-PublicProperties INTERFACE ImGui::ImGui)

# ImGuiNodeEditor
add_subdirectory(lib/imgui-node-editor)
target_include_directories(ImGuiNodeEditor SYSTEM PUBLIC lib/imgui)
target_link_libraries(Cool-PublicProperties INTERFACE ImGuiNodeEditor::ImGuiNodeEditor)
install(FILES "lib/imgui-node-editor/LICENSE" DESTINATION "license/imgui-node-editor")

# imgui_markdown
install(FILES "lib/imgui_markdown/License.txt" DESTINATION "license/imgui_markdown")

# ImStyleEd
add_subdirectory(lib/ImGui_StyleEditor)
target_include_directories(ImStyleEd SYSTEM PRIVATE lib)
target_link_libraries(Cool-PublicProperties INTERFACE ImStyleEd::ImStyleEd)

# ImGuiNotify
add_subdirectory(lib/ImGuiNotify)
target_include_directories(ImGuiNotify SYSTEM PRIVATE lib/imgui)
target_link_libraries(Cool-PublicProperties INTERFACE ImGuiNotify::ImGuiNotify)

# no-sleep
add_subdirectory(lib/no-sleep)
target_link_libraries(Cool-PublicProperties INTERFACE no_sleep::no_sleep)

# ----------------
# ---Copy files---
# ----------------
include("CMakeUtils/files_and_folders.cmake")

function(cool_setup TARGET_NAME)
    ffmpeg_copy_libs(${TARGET_NAME})
    Cool__target_copy_folder(${TARGET_NAME} ${COOL_PATH_INTERNAL}/res)
    target_copy_webgpu_binaries(${TARGET_NAME})
    install(DIRECTORY ${COOL_PATH_INTERNAL}/res DESTINATION bin/${COOL_PATH_INTERNAL})
endfunction()

# ------------------
# ---Set the app icon---
# ------------------
# Call this function if you want to set an icon for your app.
# It will be used as the executable's icon and be visible in the corner of your app's window.
# To do so you will need both a .png and .ico files containing your logo.
#
# The .ico will only be used at compile time. For more info about .ico, see https://docs.fileformat.com/image/ico/.
# It needs to be in the same folder as a .rc file containing "IDI_ICON1 ICON DISCARDABLE "your_icon.ico""
# where your_icon.ico is the name of your .ico file.
# `RC_FILE` is the path to that .rc file.
#
# The .png (or actually any image format supported by CoolLab) will be used at runtime.
# It needs to be copied alongside your executable using `Cool__target_copy_file` or `Cool__target_copy_folder`.
# `IMAGE_FILE` is the path to that .png file.
# It MUST be relative to the root of your project (a.k.a. to `Cool::Path::root()`).
function(Cool__set_app_icon
    TARGET
    IMAGE_FILE
    RC_FILE
)
    target_compile_definitions(Cool-PrivateProperties INTERFACE COOL_APP_ICON_FILE=\"${IMAGE_FILE}\")

    if(WIN32)
        target_sources(${TARGET} PRIVATE ${RC_FILE})
    endif()
endfunction()

# ---------------------
# ---Setup the tests---
# ---------------------
add_executable(Tests-Cool tests/tests.cpp ${COOL_SOURCES})
target_link_libraries(Tests-Cool PRIVATE Cool-PublicProperties)
target_link_libraries(Tests-Cool PRIVATE Cool-PrivateProperties)
target_compile_definitions(Tests-Cool PRIVATE COOL_ENABLE_TESTS)
ffmpeg_copy_libs(Tests-Cool)

set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(lib/doctest)
target_link_libraries(Tests-Cool PRIVATE doctest::doctest)
install(FILES "lib/doctest/LICENSE.txt" DESTINATION "license/doctest")

set_target_properties(Tests-Cool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})