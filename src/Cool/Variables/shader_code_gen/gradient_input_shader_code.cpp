#include "Cool/StrongTypes/Gradient.h"

namespace Cool {

static auto gen_code_wrap_mode(ImGG::WrapMode wrap_mode) -> std::string
{
    switch (wrap_mode)
    {
    case ImGG::WrapMode::Clamp:
    {
        return "clamp(x, 0., 1.);";
    }
    case ImGG::WrapMode::Repeat:
    {
        return "fract(x);";
    }
    case ImGG::WrapMode::MirrorRepeat:
    {
        return "1. - abs(mod(x, 2.) -1.);";
    }
    }
    assert(false);
    return "";
}

static auto gen_code_interpolation(std::string_view name, ImGG::Interpolation interpolation_mode) -> std::string
{
    using fmt::literals::operator""_a;
    switch (interpolation_mode)
    {
    case ImGG::Interpolation::Linear_Light:
    case ImGG::Interpolation::Linear_Paint:
    {
        return fmt::format(
            FMT_COMPILE(R"STR(
            // The color is in Oklab space with premultiplied alpha because interpolating in that space looks better (it matches human perception).
            float mix_factor = (x_wrapped - {gradient_marks}[i - 1].pos) /
                            ({gradient_marks}[i].pos - {gradient_marks}[i - 1].pos);
            vec4 col1 = {gradient_marks}[i - 1].col;
            vec4 col2 = {gradient_marks}[i    ].col;
            return {mix_colors};
    )STR"),
            "gradient_marks"_a = fmt::format("{}_", name),
            "mix_colors"_a     = interpolation_mode == ImGG::Interpolation::Linear_Light
                                     ? "mix(col1, col2, mix_factor)"
                                     : R"STR(
        vec4(
            Cool_premultiply_color(
                Cool_Oklab_from_sRGB(mixbox_lerp(
                    Cool_sRGB_from_Oklab(Cool_unpremultiply_color(col1.rgb, col1.a)),
                    Cool_sRGB_from_Oklab(Cool_unpremultiply_color(col2.rgb, col2.a)),
                    mix_factor
                )),
                mix(col1.a, col2.a, mix_factor)
            ),
            mix(col1.a, col2.a, mix_factor)
        )
                )STR"
        );
    }
    case ImGG::Interpolation::Constant:
    {
        return fmt::format(
            FMT_COMPILE(R"STR(
            return {gradient_marks}[i].col;
    )STR"),
            "gradient_marks"_a = fmt::format("{}_", name)
        );
    }
    }
    assert(false);
    return "";
}

static auto gen_code_number_of_marks_variable_name(std::string_view name)
{
    return fmt::format("numberOfMarksOf{}", internal::gradient_marks_array_name(name));
}

auto gradient_input_shader_code(Cool::Gradient const& value, std::string_view name) -> std::string
{
    using fmt::literals::operator""_a;
    return value.value.gradient().is_empty()
               ? fmt::format(
                     R"STR(
vec4 {}(float x)   
{{
    return vec4(0.);
}}
         )STR",
                     name
                 )
               : fmt::format(
                     FMT_COMPILE(R"STR( 
#include "_COOL_RES_/shaders/GradientMark.glsl"
const int {number_of_marks} = {gradient_size};
uniform GradientMark {gradient_marks}[{number_of_marks}];

vec4 {gradient_function}(float x) // we benchmarked and linear scan is faster that dichotomy
{{
    float x_wrapped = {wrap};
    if (x_wrapped <= {gradient_marks}[0].pos)
    {{
        return {gradient_marks}[0].col;
    }}
    for (int i = 1; i < {number_of_marks}; i++)
    {{
        if ((x_wrapped <= {gradient_marks}[i].pos) && (x_wrapped >= {gradient_marks}[i - 1].pos))
        {{
            {interpolation}
        }}
    }}
    if (x_wrapped >= {gradient_marks}[{number_of_marks} - 1].pos)
    {{
        return {gradient_marks}[{number_of_marks} - 1].col;
    }}
}}
    )STR"),
                     "gradient_size"_a     = value.value.gradient().get_marks().size(),
                     "number_of_marks"_a   = gen_code_number_of_marks_variable_name(name),
                     "gradient_function"_a = name,
                     "wrap"_a              = gen_code_wrap_mode(value.wrap_mode),
                     "interpolation"_a     = gen_code_interpolation(name, value.value.gradient().interpolation_mode()),
                     "gradient_marks"_a    = internal::gradient_marks_array_name(name)
                 );
}

} // namespace Cool