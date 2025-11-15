#include "Cool/StrongTypes/MathExpression.h"

namespace Cool {

auto math_expression_input_shader_code(Cool::MathExpression const& value, std::string_view name) -> std::string
{
    using fmt::literals::operator""_a;
    return fmt::format(
        FMT_COMPILE(
            R"STR(
float {name}(float x)
{{
    return {expression};
}}
            )STR"
        ),
        "expression"_a = value.expression,
        "name"_a       = name
    );
}

} // namespace Cool