#pragma once

#include <Cool/ImGui/icon_fmt.h>
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "DebugOptions.h"

namespace Cool {

// https://stackoverflow.com/questions/16387354/template-tuple-calling-a-function-on-each-element/37100197#comment84411734_37100197
#define COOL_DEBUG_OPTIONS_MANAGER_FOR_ALL_T(fn_name) std::apply([](auto&&... x) { (static_cast<void>(fn_name(std::forward<decltype(x)>(x))), ...); }, std::tuple<Ts...>{})

template<typename... Ts>
class DebugOptionsManager {
public:
    static void imgui_ui_for_all_options()
    {
        if (ImGui::IsWindowAppearing())
        {
            reset_filter();
            ImGui::SetKeyboardFocusHere();
        }
        if (ImGui::InputTextWithHint(
                "##Filter", icon_fmt("Filter debug options", ICOMOON_SEARCH).c_str(), &instance().filter,
                ImGuiInputTextFlags_EnterReturnsTrue
            ))
        {
            toggle_first_option();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset all debug options"))
        {
            reset_all();
        }
        ImGui::Separator();
        COOL_DEBUG_OPTIONS_MANAGER_FOR_ALL_T(imgui_ui_for_all_options_impl);
    }

    static void reset_filter()
    {
        instance().filter = "";
    }

    static void reset_all()
    {
        reset_filter();
        COOL_DEBUG_OPTIONS_MANAGER_FOR_ALL_T(reset_all_impl);
    }

    static void update()
    {
        COOL_DEBUG_OPTIONS_MANAGER_FOR_ALL_T(update_impl);
    }

private:
    struct Instance {
        std::string filter;
    };

    static auto instance() -> Instance&
    {
        {
            static auto the_instance = Instance{};
            return the_instance;
        }
    }

    static void toggle_first_option()
    {
        try
        {
            COOL_DEBUG_OPTIONS_MANAGER_FOR_ALL_T(toggle_first_option_impl); // This will throw on the first option that passes the filter, to prevent to rest of the loop from happening.
        }
        catch (float)
        {}
    }

    template<typename T>
    static void toggle_first_option_impl(T&&)
    {
        T::toggle_first_option(instance().filter);
    }

    template<typename T>
    static void imgui_ui_for_all_options_impl(T&&)
    {
        T::imgui_ui_for_all_options(instance().filter);
    }

    template<typename T>
    static void reset_all_impl(T&&)
    {
        T::reset_all();
    }

    template<typename T>
    static void update_impl(T&&)
    {
        T::update();
    }
};

} // namespace Cool
