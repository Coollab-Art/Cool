#include "run.h"
#include <exception>
#include <filesystem>
#include <memory>
#include <scope_guard/scope_guard.hpp>
#include "Audio/Audio.hpp"
#include "Cool/AppManager/AppManager.h"
#include "Cool/AppManager/IApp.h"
#include "Cool/AppManager/internal/get_app_manager.hpp"
#include "Cool/CommandLineArgs/CommandLineArgs.h"
#include "Cool/Core/set_utf8_locale.hpp"
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/File/File.h"
#include "Cool/Icons/Icons.h"
#include "Cool/ImGui/ColorThemes.h"
#include "Cool/ImGui/StyleEditor.h"
#include "Cool/Log/boxer_show.hpp"
#include "Cool/Log/file_logger.hpp"
#include "Cool/Log/redirect_cout_cerr_to_log_file.hpp"
#include "Cool/Log/redirect_notifications_to_log_file.hpp"
#include "Cool/Log/reset_log_file_if_not_already_reset.hpp"
#include "Cool/Path/Path.h"
#include "Cool/Serialization/JsonSerializer.hpp"
#include "Cool/TextureSource/TextureLibrary_Webcam.hpp"
#include "Cool/UserSettings/UserSettings.h"
#include "Cool/View/ViewsManager.h"
#include "Cool/Window/internal/WindowFactory.h"
#include "ImGuiNotify/ImGuiNotify.hpp"

#if defined(COOL_VULKAN)
#include <Cool/Gpu/Vulkan/Context.h>
#endif

namespace Cool::internal {

static auto create_autosaver(std::function<void()> const& save) -> std::function<void()>
{
    // This function will be run every frame
    return [save]() {
        if (!user_settings().autosave_enabled)
            return;

        static auto last_time = std::chrono::steady_clock::now();
        const auto  now       = std::chrono::steady_clock::now();
        if (Time{now - last_time} > user_settings().autosave_delay)
        {
            save();
            last_time = now;
            if (DebugOptions::log_when_autosaving())
                Log::info("Autosave", "The application was just saved.");
        }
    };
}

/// Makes sure the user_data() folder is populated with all the initial_user_data() files
static void copy_initial_user_data_ifn()
{
    if (!Cool::File::exists(Cool::Path::initial_user_data()))
        return;
    try
    {
        for (auto const& entry : std::filesystem::recursive_directory_iterator(Cool::Path::initial_user_data()))
        {
            auto const& default_path = entry.path();
            if (!Cool::File::is_regular_file(default_path))
                continue;

            auto const path = Cool::Path::user_data() / Cool::File::relative(default_path, Cool::Path::initial_user_data());
            if (Cool::File::exists(path))
                continue;

            try
            {
                Cool::File::copy_file(default_path, path);
            }
            catch (std::exception const& e)
            {
                Log::internal_warning("Initial user data", fmt::format("Failed to copy \"{}\" to \"{}\":\n{}", Cool::File::weakly_canonical(default_path), Cool::File::weakly_canonical(path), e.what()));
            }
        }
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("Initial user data", fmt::format("Failed to copy initial user data:\n{}", e.what()));
    }
}

void run_impl(
    int argc, char** argv, RunConfig const& config,
    std::function<void()> const&                                                                         initialize_paths_config,
    std::function<std::unique_ptr<IApp>(Cool::WindowManager& windows, Cool::ViewsManager& views)> const& make_unique_app
)
{
    try
    {
        set_utf8_locale();
        command_line_args().init(argc, argv);
        initialize_paths_config();
        start_redirecting_cout_cerr_to_log_file(); // Can only be done once the paths config is initialized, otherwise we won't know where to create the log file
        redirect_notifications_to_log_file();
        reset_log_file_if_not_already_reset();
        copy_initial_user_data_ifn();
        // Create window
        // TODO(WebGPU) check that if an exception is thrown during window creation, ImGui shuts down properly and doesn't prevent us from logging the error to the log file
        init_backend_context(config.window_config);

        ImStyleEd::error_handlers().on_mandatory_warning = [](std::string_view error_message) {
            ImGuiNotify::send({
                .type    = ImGuiNotify::Type::Warning,
                .title   = "Color Themes",
                .content = std::string{error_message},
            });
        };
        ImStyleEd::error_handlers().on_optional_warning = [](std::string_view error_message) {
            Log::internal_warning("Color Themes", std::string{error_message});
        };
        style_editor().emplace(); // Make sure we load all the ImGui style settings // Done after the creation of the windows because we need an ImGui context to set its Style
        color_themes().emplace(); // Make sure we load all the ImGui color settings // Done after the creation of the windows because we need an ImGui context to set its Style

        Icons::close_button(); // Make sure the MessageConsole won't deadlock at startup when the "Log when creating textures" option is enabled (because displaying the console requires the close_button, which will generate a log when its texture gets created).

        // Init error callbacks
        Audio::set_error_callback([](RtAudioErrorType /* type */, std::string const& error_message) {
            Log::internal_warning("Audio", error_message);
        });

        // Create the App
        auto views = ViewsManager{};
        auto app   = make_unique_app(views); // Stored in a unique_ptr because we don't require App to be copy-assignable

        auto app_serializer = JsonSerializer{
            "last_session.json",
            [&](nlohmann::json const& json) {
                app->load_from_json(json);
            },
            [&](nlohmann::json& json) {
                app->save_to_json(json);
            },
            false /*use_shared_user_data*/,
        };
        app_serializer.load();
        auto const save_on_exit = sg::make_scope_guard([&]() {
            app_serializer.save();
            DebugOptions::save();
            user_settings().save(); // Even if the user doesn't change the settings, we will save the settings they have seen once, so that if a new version of the software comes with new settings, we will not change settings that the user is used to
            // style_editor().save(); // If the user doesn't change the style, we don't want to save it, so that if a new version of the software comes with a new style, if the user hasn't customized the style then we will use the new style from user_data_default
        });

        // Run the app
        auto app_manager  = Cool::AppManager{views, *app, config.app_manager_config};
        get_app_manager() = &app_manager;
        app_manager.run(internal::create_autosaver([&]() {
            app_serializer.save();
        }));

        // Shutdown
        get_app_manager() = nullptr;
        app->on_shutdown();
    }
    catch (std::exception const& e)
    {
        boxer_show(e.what(), COOL_APP_NAME " Error", boxer::Style::Error);
    }

    // Global Shutdown
    style_editor().reset(); // Destroy it to make sure it saves now, before the ImGui context is destroyed, otherwise it wouldn't be able to access the ImGuiStyle anymore
    color_themes().reset(); // Destroy it to make sure it saves now, before the ImGui context is destroyed, otherwise it wouldn't be able to access the ImGuiStyle anymore
    Audio::shut_down();
    TextureLibrary_Webcam::instance().shut_down(); // We must destroy the textures in the WebcamImages before the texture_pool() gets destroyed
    stop_redirecting_cout_cerr_to_log_file();      // Need to do this explicitly, otherwise we will sync the cout buffers during static deinitialization, and the file_logger might have already been destroyed
}

} // namespace Cool::internal