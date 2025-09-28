# ---HOW TO---

# ------------

from dataclasses import dataclass
from enum import Enum, auto


class Kind(Enum):
    CHECKBOX = auto()
    WINDOW = auto()
    BUTTON = auto()


@dataclass
class DebugOption:
    name_in_code: str
    name_in_ui: str
    available_in_release: bool
    # If None, this will default to `name_in_ui`. You might want to change the `name_in_ui` at some point, but keep the `name_in_json` the same so that you don't break serialization.
    name_in_json: str | None = None
    window_name: str = ""
    kind: Kind = Kind.CHECKBOX
    default_value_in_debug: bool = False
    default_value_in_release: bool = False
    detailed_description: str = ""


def build_type(code: str, debug_option: DebugOption) -> str:
    if debug_option.available_in_release:
        return code
    else:
        return f"""#if DEBUG
{code}
#endif"""


def default_value(debug_option: DebugOption):
    if (
        not debug_option.available_in_release
        or debug_option.default_value_in_release == debug_option.default_value_in_debug
    ):
        return "true" if debug_option.default_value_in_debug else "false"
    return f"""
#if defined(DEBUG)
{"true" if debug_option.default_value_in_debug else "false"}
#else
{"true" if debug_option.default_value_in_release else "false"}
#endif
"""


def debug_options_variables(debug_options: list[DebugOption]):
    return "\n".join(
        map(
            lambda debug_option: build_type(
                f"bool {debug_option.name_in_code}{{{default_value(debug_option)}}};",
                debug_option,
            ),
            debug_options,
        )
    )


def window_name(debug_option: DebugOption):
    return (
        debug_option.window_name
        if debug_option.window_name
        else debug_option.name_in_ui
    )


def option_implementation(debug_option: DebugOption):
    def code():
        match debug_option.kind:
            case Kind.CHECKBOX | Kind.BUTTON:
                return f"[[nodiscard]] static auto {debug_option.name_in_code}() -> bool& {{ return instance().{debug_option.name_in_code}; }}"
            case Kind.WINDOW:
                return f"""static void {debug_option.name_in_code}(std::function<void()> callback)
                {{
                    if (instance().{debug_option.name_in_code}) 
                    {{
                        ImGui::Begin(Cool::icon_fmt("{window_name(debug_option)}", ICOMOON_WRENCH).c_str(), &instance().{debug_option.name_in_code}, ImGuiWindowFlags_NoFocusOnAppearing);
                        callback();
                        ImGui::End();
                        if (!instance().{debug_option.name_in_code}) // Window has just been closed manually by the user
                            save();
                    }}
                }}"""
            case _:
                return ""

    return build_type(code(), debug_option)


def imgui_widget(debug_option: DebugOption):
    match debug_option.kind:
        case Kind.CHECKBOX | Kind.WINDOW:
            return f"""
            if (Cool::ImGuiExtras::toggle("{debug_option.name_in_ui}", &instance().{debug_option.name_in_code}))
                save();
            """
        case Kind.BUTTON:
            return f"""
            instance().{debug_option.name_in_code} = ImGui::Button("##{debug_option.name_in_ui}", {{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}});
            ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text("{debug_option.name_in_ui}");
            if (ImGui::IsItemClicked())
                instance().{debug_option.name_in_code} = true;
            """


def options_implementations(debug_options: list[DebugOption]):
    return "\n".join(map(option_implementation, debug_options))


def passes_the_filter(debug_option: DebugOption):
    return f'wafl::similarity_match({{filter, "{debug_option.name_in_ui}"}}) >= wafl::Matches::Strongly'


def imgui_ui_for_all_options(debug_options: list[DebugOption]):
    return "\n".join(
        map(
            lambda debug_option: build_type(
                f"""
                        if ({passes_the_filter(debug_option)})
                        {{
                            {imgui_widget(debug_option)}
                            {f'Cool::ImGuiExtras::help_marker("{debug_option.detailed_description}");' if debug_option.detailed_description else ""}
                        }}
                        """,
                debug_option,
            ),
            debug_options,
        )
    )


def toggle_first_option(debug_options: list[DebugOption]):
    return "\n".join(
        map(
            lambda debug_option: build_type(
                f"""
                        if ({passes_the_filter(debug_option)})
                        {{
                            instance().{debug_option.name_in_code} = !instance().{debug_option.name_in_code};
                            save();
                            throw 0.f; // To understand why we need to throw, see `toggle_first_option()` in <Cool/DebugOptions/DebugOptionsManager.h>
                        }}
                        """,
                debug_option,
            ),
            debug_options,
        )
    )


def filter_out_buttons(debug_options: list[DebugOption]):
    return filter(lambda dbg: dbg.kind != Kind.BUTTON, debug_options)


def json_serialization(debug_options: list[DebugOption], set_or_get: str):
    def code(options):
        return (
            ";\n".join(
                map(
                    lambda debug_option: f'Cool::json_{set_or_get}(json, "{debug_option.name_in_json or debug_option.name_in_ui}", {debug_option.name_in_code})',
                    filter_out_buttons(options),
                )
            )
            + ";"
        )

    debug = code(debug_options)
    release = code(filter(lambda dbg: dbg.available_in_release, debug_options))

    return f"""
#if DEBUG
{debug}
#else
{release}
#endif
"""


def reset_all(debug_options: list[DebugOption]):
    return "\n".join(
        map(
            lambda debug_option: build_type(
                f"instance().{debug_option.name_in_code} = {default_value(debug_option)};",
                debug_option,
            ),
            filter_out_buttons(debug_options),
        )
    )


def DebugOptions(
    debug_options: list[DebugOption],
    namespace: str,
    cache_file_name: str,
    wants_to_log_warnings: str,
):
    return f"""
#include "Cool/ImGui/ImGuiExtras.h"
#include "wafl/wafl.hpp"
#include "Cool/Serialization/JsonAutoSerializer.hpp"
#include "Cool/Serialization/Json.hpp"
#include "Cool/Path/Path.h"

namespace Cool {{
    template<typename... Ts>
    class DebugOptionsManager; // Forward declare this class so that the friend declaration that comes later on doesn't fail.
}}

namespace {namespace} {{

class DebugOptions {{
public:
    {options_implementations(debug_options)}

    static void save() {{ instance()._serializer.save(); }}

private:
    struct Instance {{        
        {debug_options_variables(debug_options)}
        
        // Must be declared last, after all the variables it serializes, so that the values it loads overwrite the default values, and not the other way around
        Cool::JsonAutoSerializer _serializer{{ 
            "{cache_file_name}.json",
            false /*autosave_when_destroyed*/, // This is a static instance, so saving it in the destructor is dangerous because we don't know when it will happen exactly. Instead, we call save manually at the end of the run()
            [&](nlohmann::json const& json) {{
                {json_serialization(debug_options, "get")}
            }},
            [&](nlohmann::json& json) {{
                {json_serialization(debug_options, "set")}
            }},
            false /*use_shared_user_data*/,
            {wants_to_log_warnings}
        }};
    }};

    static auto instance() -> Instance&
    {{
        static auto the_instance = Instance{{}};
        return the_instance;
    }}
    
    static void load() {{instance()._serializer.load();}}
    static void update() {{instance()._serializer.update();}}

    static void reset_all()
    {{
        {reset_all(debug_options)}
        save();
    }}

    template<typename... Ts>
    friend class Cool::DebugOptionsManager; // We go through this indirection so that only the files which include "DebugOptionsManager" can call `imgui_ui_for_all_options()`
    
    static void imgui_ui_for_all_options(std::string_view filter)
    {{
        {imgui_ui_for_all_options(debug_options)}
    }}

    static void toggle_first_option(std::string_view filter)
    {{
        {toggle_first_option(debug_options)}
    }}
}};

}} // namespace {namespace}
"""


def generate_debug_options(
    output_folder: str,
    namespace: str,
    cache_file_name: str,
    debug_options: list[DebugOption],
    wants_to_log_warnings: str = "Cool::WantsToLogWarnings::CheckInDebugOption",
):
    import os
    from pathlib import Path
    from importlib.machinery import SourceFileLoader

    generate_files = SourceFileLoader(
        "generate_files",
        os.path.join(
            Path(os.path.abspath(__file__)).parent.parent.parent.parent,
            "tooling",
            "generate_files.py",
        ),
    ).load_module()

    def fn():
        return DebugOptions(
            debug_options=debug_options,
            namespace=namespace,
            cache_file_name=cache_file_name,
            wants_to_log_warnings=wants_to_log_warnings,
        )

    fn.__name__ = "DebugOptions"

    generate_files.generate(
        folder=output_folder,
        files=[
            fn,
        ],
    )
