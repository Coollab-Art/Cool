# ---HOW TO---
# Create a function with the same name as the file you want to generate
# that returns a string with the content you want to generate.
#
# Then add your function to the call to `generate()` at the end of the file.
#
# You can use `all_variable_types()` to get all the variable types we use in Coollab.
# ------------

from dataclasses import dataclass, field
from typing import List

if True:
    # HACK: Python doesn't allow us to import from a parent folder (e.g. tooling.generate_files)
    # So we need to add the path manually to sys.path
    import os
    import sys
    from pathlib import Path

    sys.path.append(
        os.path.join(Path(os.path.abspath(__file__)).parent.parent, "ColorSpaces")
    )
    # End of HACK
    import generator_colors


@dataclass
class VariableMetadata:
    field_name: str
    pretty_name: str
    type: str
    default_value: str


@dataclass
class VariableDescription:
    input_type: List[str]  # Type that comes in an INPUT declaration
    cpp_type: str  # Type used in C++ to store the type
    glsl_type: str  # Raw glsl used to store the type
    include: str = ""  # File containing the C++ type
    metadatas: list[VariableMetadata] = field(default_factory=lambda: [])
    do_generate_get_default_metadata: bool = True
    # For example a MathExpression can't be sent as a simple uniform to a shader; that's why we generate some code to inject it into the shader.
    # For more complex cases like Gradient which only sometimes requires code generation, set this to False and hardcode something as the `use_secondary_dirty_flag` when calling set_dirty() (e.g., if you are developing Coollab, in `set_value_default_impl()` of Command_SetVariable.h)
    always_requires_shader_code_generation: bool = False


def angle_metadatas():
    return [
        VariableMetadata(
            field_name="number_of_snaps",
            pretty_name="Number of snaps",
            type="int",
            default_value="24",
        ),
        VariableMetadata(
            field_name="snaps_offset",
            pretty_name="Snaps offset (in radians)",
            type="float",
            default_value="0.f",
        ),
        VariableMetadata(
            field_name="always_snap",
            pretty_name="Always snap",
            type="bool",
            default_value="false",
        ),
    ]


def float_metadatas(is_double=False):
    return [
        VariableMetadata(
            field_name="bounds",
            pretty_name="Bounds",
            type=f"internal::BoundsMetadata<{"double" if is_double else "float"}>",
            default_value=f"""
                .min = 0.{"" if is_double else "f"},
                .max = 1.{"" if is_double else "f"},
                .has_min_bound = false,
                .has_max_bound = false,
                .drag_speed = 0.01f,
                .use_slider = false,
                .is_logarithmic = false,
            """,
        ),
    ]


def hdr_metadata():
    return VariableMetadata(
        field_name="is_hdr",
        pretty_name="Is HDR",
        type="bool",
        default_value="false",
    )


def all_variable_descriptions():
    return [
        VariableDescription(
            input_type=["bool"],
            cpp_type="bool",
            glsl_type="bool",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["int"],
            cpp_type="int",
            glsl_type="int",
            metadatas=[
                VariableMetadata(
                    field_name="bounds",
                    pretty_name="Bounds",
                    type="internal::BoundsMetadata<int>",
                    default_value="""
                        .min = 0,
                        .max = 12,
                        .has_min_bound = false,
                        .has_max_bound = false,
                        .drag_speed = 0.02f,
                        .use_slider = false,
                        .is_logarithmic = false,
                    """,
                ),
            ],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["float"],
            cpp_type="float",
            glsl_type="float",
            metadatas=float_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["UV", "Point2D"],
            cpp_type="Cool::Point2D",
            glsl_type="vec2",
            include="<Cool/StrongTypes/Point2D.h>",
            metadatas=[
                VariableMetadata(
                    field_name="drag_speed",
                    pretty_name="Drag speed",
                    type="float",
                    default_value="0.0001f",
                ),
            ],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["vec2"],
            cpp_type="glm::vec2",
            glsl_type="vec2",
            include="<glm/glm.hpp>",
            metadatas=float_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["vec3"],
            cpp_type="glm::vec3",
            glsl_type="vec3",
            include="<glm/glm.hpp>",
            metadatas=float_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["vec4"],
            cpp_type="glm::vec4",
            glsl_type="vec4",
            include="<glm/glm.hpp>",
            metadatas=float_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=list(
                map(lambda x: x.name_in_code, generator_colors.color_spaces())
            ),
            cpp_type="Cool::Color",
            glsl_type="vec3",
            include="<Cool/StrongTypes/Color.h>",
            metadatas=[
                hdr_metadata(),
            ],
            do_generate_get_default_metadata=False,
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=list(generator_colors.color_and_alpha_spaces_names()),
            cpp_type="Cool::ColorAndAlpha",
            glsl_type="vec4",
            include="<Cool/StrongTypes/ColorAndAlpha.h>",
            metadatas=[
                hdr_metadata(),
            ],
            do_generate_get_default_metadata=False,
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Camera"],
            cpp_type="Cool::Camera",
            # NB: we would probably need to create a Camera struct in glsl if we really intended to use this variable in shaders. (Which we will definitely do at some point instead of having one single global camera)
            glsl_type="mat4",
            include="<Cool/Camera/Camera.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Camera2D"],
            cpp_type="Cool::Camera2D",
            glsl_type="mat3",
            include="<Cool/StrongTypes/Camera2D.h>",
            metadatas=angle_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Angle"],
            cpp_type="Cool::Angle",
            glsl_type="float",
            include="<Cool/StrongTypes/Angle.h>",
            metadatas=angle_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Direction2D"],
            cpp_type="Cool::Direction2D",
            glsl_type="vec2",
            include="<Cool/StrongTypes/Direction2D.h>",
            metadatas=angle_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Hue"],
            cpp_type="Cool::Hue",
            glsl_type="float",
            include="<Cool/StrongTypes/Hue.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["ColorPalette"],
            cpp_type="Cool::ColorPalette",
            glsl_type="NO TYPE THIS IS A FUNCTION",
            include="<Cool/StrongTypes/ColorPalette.h>",
            metadatas=[
                hdr_metadata(),
            ],
            always_requires_shader_code_generation=True,  # TODO Should be false and only require shader generation when changing the number of colors
        ),
        VariableDescription(
            input_type=["Gradient"],
            cpp_type="Cool::Gradient",
            glsl_type="NO TYPE THIS IS A FUNCTION",
            include="<Cool/StrongTypes/Gradient.h>",
            metadatas=[
                hdr_metadata(),
                VariableMetadata(
                    field_name="randomize_new_marks_colors",
                    pretty_name="Randomize new marks' colors",
                    type="bool",
                    default_value="true",
                ),
            ],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Texture"],
            cpp_type="Cool::TextureDescriptor_Image",
            glsl_type="Cool_Texture",
            include="<Cool/TextureSource/TextureDescriptor.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Webcam"],
            cpp_type="Cool::TextureDescriptor_Webcam",
            glsl_type="Cool_Texture",
            include="<Cool/TextureSource/TextureDescriptor.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["MathExpression"],
            cpp_type="Cool::MathExpression",
            glsl_type="NO TYPE THIS IS A FUNCTION",
            include="<Cool/StrongTypes/MathExpression.h>",
            metadatas=[],
            always_requires_shader_code_generation=True,
        ),
        VariableDescription(
            input_type=["mat2"],
            cpp_type="glm::mat2",
            glsl_type="mat2",
            include="<glm/glm.hpp>",
            metadatas=float_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["mat3"],
            cpp_type="glm::mat3",
            glsl_type="mat3",
            include="<glm/glm.hpp>",
            metadatas=float_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["mat4"],
            cpp_type="glm::mat4",
            glsl_type="mat4",
            include="<glm/glm.hpp>",
            metadatas=float_metadatas(),
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Midi"],
            cpp_type="Cool::MidiChannel",
            glsl_type="float",
            include="<Cool/Midi/MidiChannel.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["OSC"],
            cpp_type="Cool::OSCChannel",
            glsl_type="float",
            include="<Cool/OSC/OSCChannel.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["TimeSpeed"],
            cpp_type="Cool::TimeSpeed",
            glsl_type="float",
            include="<Cool/Time/TimeSpeed.h>",
            metadatas=[
                VariableMetadata(
                    field_name="min",
                    pretty_name="Min",
                    type="double",
                    default_value="0.",
                ),
                VariableMetadata(
                    field_name="max",
                    pretty_name="Max",
                    type="double",
                    default_value="1.",
                ),
                VariableMetadata(
                    field_name="has_min_bound",
                    pretty_name="Has min bound",
                    type="bool",
                    default_value="false",
                ),
                VariableMetadata(
                    field_name="has_max_bound",
                    pretty_name="Has max bound",
                    type="bool",
                    default_value="false",
                ),
                VariableMetadata(
                    field_name="drag_speed",
                    pretty_name="Drag speed",
                    type="float",
                    default_value="0.001f",
                ),
            ],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Video"],
            cpp_type="Cool::TextureDescriptor_Video",
            glsl_type="Cool_Texture",
            include="<Cool/TextureSource/TextureDescriptor.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["Time"],
            cpp_type="Cool::Time",
            glsl_type="float",
            include="<Cool/Time/Time.hpp>",
            metadatas=[
                VariableMetadata(
                    field_name="min",
                    pretty_name="Min",
                    type="double",
                    default_value="0.",
                ),
                VariableMetadata(
                    field_name="max",
                    pretty_name="Max",
                    type="double",
                    default_value="10.",
                ),
                VariableMetadata(
                    field_name="has_min_bound",
                    pretty_name="Has min bound",
                    type="bool",
                    default_value="false",
                ),
                VariableMetadata(
                    field_name="has_max_bound",
                    pretty_name="Has max bound",
                    type="bool",
                    default_value="false",
                ),
                VariableMetadata(
                    field_name="drag_speed",
                    pretty_name="Drag speed",
                    type="float",
                    default_value="0.01f",
                ),
                VariableMetadata(
                    field_name="show_milliseconds",
                    pretty_name="Show milliseconds",
                    type="bool",
                    default_value="true",
                ),
            ],
            always_requires_shader_code_generation=False,
        ),
        VariableDescription(
            input_type=["SpoutSyphon"],
            cpp_type="Cool::TextureDescriptor_SpoutSyphon",
            glsl_type="Cool_Texture",
            include="<Cool/TextureSource/TextureDescriptor.h>",
            metadatas=[],
            always_requires_shader_code_generation=False,
        ),
    ]


def all_variable_types():
    return map(lambda desc: desc.cpp_type, all_variable_descriptions())


def all_types_representations_as_strings():
    return {desc.cpp_type: desc.input_type for desc in all_variable_descriptions()}


def strip_namespace(variable_type):
    return variable_type[variable_type.rfind(":") + 1 :]


def all_variable_types_without_namespaces():
    return map(
        lambda variable_type: strip_namespace(variable_type), all_variable_types()
    )


def all_variable_descriptions_without_namespaces():
    def strip_namespace_on_variable_type(desc):
        desc.type = strip_namespace(desc.type)
        return desc

    return map(
        lambda variable_type_and_metadatas: strip_namespace_on_variable_type(
            variable_type_and_metadatas
        ),
        all_variable_descriptions(),
    )


def all_variable_includes():
    return f"""
            #include <Cool/Camera/Camera.h>
            #include <Cool/Variables/Variables.h>
            """


def all_includes():
    return map(lambda var_desc: var_desc.include, all_variable_descriptions())


def all_non_empty_includes():
    return filter(lambda include: include != "", all_includes())


def all_types_includes():
    return "\n".join(map(lambda path: f"#include {path}", all_non_empty_includes()))


# def register_set_variable_commands():
#     commands = ""
#     reversible_commands = ""
#     for variable_type in all_variable_types():
#         commands += f"LAB_REGISTER_COMMAND(Lab::Command_SetVariable<{variable_type}>)\n"
#         reversible_commands += f"LAB_REGISTER_REVERSIBLE_COMMAND(Lab::ReversibleCommand_SetVariable<{variable_type}>)\n"
#     return f"""
# {commands}
# {reversible_commands}
# """


# def register_set_variable_metadata_commands():
#     out = "\n"
#     for variable_type in all_variable_types():
#         out += f"LAB_REGISTER_COMMAND(Lab::Command_SetVariableMetadata<{variable_type}>)\n"
#     return out


def AnySharedVariable():
    return (
        "\n"
        + "using AnySharedVariable = std::variant<\n"
        + ",\n".join(
            map(lambda var_type: f"SharedVariable<{var_type}>", all_variable_types())
        )
        + "\n>;"
    )


def AnySharedVariableDefinition():
    return (
        "\n"
        + "using AnySharedVariableDefinition = std::variant<\n"
        + ",\n".join(
            map(
                lambda var_type: f"SharedVariableDefinition<{var_type}>",
                all_variable_types(),
            )
        )
        + "\n>;"
    )


def AnyVariableData():
    return ",\n".join(
        map(lambda var_type: f"VariableData<{var_type}>", all_variable_types())
    )


def AnyVariable():
    return ",\n".join(
        map(lambda var_type: f"Variable<{var_type}>", all_variable_types())
    )


def variables_includes():
    out = "\n"
    for variable_type in all_variable_types_without_namespaces():
        out += f"#include <Cool/Variables/Variable_{variable_type}.h>\n"
    return out


def glsl_type():
    return "\n\n".join(
        map(
            lambda desc: f"""
            template<>
            auto glsl_type<{desc.cpp_type}>() -> std::string {{ 
                return "{desc.glsl_type}";
            }}
            """,
            all_variable_descriptions(),
        )
    )


def ser20_make_nvp(metadatas: List[VariableMetadata]):
    return ",\n".join(
        map(
            lambda meta: f'ser20::make_nvp("{meta.pretty_name}", {meta.field_name})',
            metadatas,
        )
    )


def ser20_serialize_body(metadatas: List[VariableMetadata]):
    return f"""
        archive(
{ser20_make_nvp(metadatas)}
        );
"""


def metadatas_definitions(metadatas: List[VariableMetadata]):
    return "\n".join(
        map(
            lambda metadata: f"{metadata.type} {metadata.field_name}{{{metadata.default_value}}};",
            metadatas,
        )
    )


def variable_definition_factory(variable_type_and_metadatas):
    def variable_definition():
        has_metadatas = len(variable_type_and_metadatas.metadatas) > 0
        return f"""
            {f'#include {variable_type_and_metadatas.include}' if variable_type_and_metadatas.include else ""}
            #include <Cool/Variables/Variable.h>
            #include <Cool/Variables/internal/BoundsMetadata.h>

            namespace Cool {{

            template<>
            struct VariableMetadata<{variable_type_and_metadatas.cpp_type}> {{
                {metadatas_definitions(variable_type_and_metadatas.metadatas)}

                friend auto operator<=>(VariableMetadata<{variable_type_and_metadatas.cpp_type}> const&, VariableMetadata<{variable_type_and_metadatas.cpp_type}> const&) = default;

            private:
                // Serialisation
                friend class ser20::access;
                template<class Archive>
                void serialize(Archive&{" archive" if has_metadatas else ""})
                {{{ser20_serialize_body(variable_type_and_metadatas.metadatas) if has_metadatas else ""}
                }}
            }};

            auto imgui_widget(Variable<{variable_type_and_metadatas.cpp_type}>&) -> bool;
            auto imgui_widget(VariableMetadata<{variable_type_and_metadatas.cpp_type}>&) -> bool;

            }} // namespace Cool
        """

    return variable_definition


def always_requires_shader_code_generation():
    return "\n\n".join(
        map(
            lambda desc: f"""
            if constexpr (std::is_same_v<T, {desc.cpp_type}>)
                return {"true" if desc.always_requires_shader_code_generation else "false"};
            """,
            all_variable_descriptions(),
        )
    )


def files():
    res = [
        # register_set_variable_commands,
        # register_set_variable_metadata_commands,
        AnySharedVariable,
        AnySharedVariableDefinition,
        AnyVariableData,
        AnyVariable,
        all_types_includes,
        all_variable_includes,
        variables_includes,
        glsl_type,
        always_requires_shader_code_generation,
    ]
    for variable_types_and_metadatas in all_variable_descriptions():
        variable_definition = variable_definition_factory(variable_types_and_metadatas)
        variable_definition.__name__ = (
            f"Variable_{strip_namespace(variable_types_and_metadatas.cpp_type)}"
        )
        res.append(variable_definition)
    return res


def main():
    # HACK: Python doesn't allow us to import from a parent folder (e.g. tooling.generate_files)
    # So we need to add the path manually to sys.path
    import os
    import sys
    from pathlib import Path

    sys.path.append(
        os.path.join(
            Path(os.path.abspath(__file__)).parent.parent.parent.parent, "tooling"
        )
    )
    # End of HACK

    import generate_files

    generate_files.generate(
        folder="generated",
        files=files(),
    )

    # HACK: Python doesn't allow us to import from a parent folder
    # So we need to load it manually
    from os import path
    from pathlib import Path
    from importlib.machinery import SourceFileLoader

    tfs = SourceFileLoader(
        "tfs",
        path.join(
            Path(path.abspath(__file__)).parent.parent,
            "type_from_string/generator_tfs.py",
        ),
    ).load_module()
    # End of HACK
    tfs.main(
        all_types_representations_as_strings(),
        all_types_includes(),
    )


if __name__ == "__main__":
    main()
