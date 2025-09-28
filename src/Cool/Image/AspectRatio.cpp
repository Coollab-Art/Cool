#include "AspectRatio.h"
#include <cmath>
#include <regex>
#include <smart/smart.hpp>
#include <stringify/stringify.hpp>
#include "Cool/ImGui/ImGuiExtras.h"

namespace Cool {

static auto make_valid_ratio(float ratio) -> float
{
    return smart::keep_above(0.001f, ratio);
}

static auto parse_ratio(std::string const& input) -> float
{
    if (input == "A4 Vertical")
        return 0.70710678f;
    if (!input.empty() && (input[0] == 'A' || input[0] == 'a')) // "A4 Horizontal"
        return 1.41421356f;

    float value = 1.f;

    std::regex const pattern(R"((\d*(\.\d*)?)(/(\d*(\.\d*)?))?(.*))"); // Matches a number and maybe a / and maybe another number
    std::smatch      matches;
    if (std::regex_match(input, matches, pattern))
    {
        if (matches.size() >= 4)
        {
            if (matches[1].matched)
            {
                if (matches[1].str() != "." && !matches[1].str().empty())
                    value = std::stof(matches[1].str());
            }
            if (matches[4].matched)
                if (matches[4].str() != "." && !matches[4].str().empty())
                    value /= std::stof(matches[4].str());
        }
    }

    return make_valid_ratio(value);
}

AspectRatio::AspectRatio(float aspect_ratio)
    : _ratio{make_valid_ratio(aspect_ratio)}
    , _input{string_from_ratio(aspect_ratio)}
{}

void AspectRatio::set(float aspect_ratio)
{
    _ratio = make_valid_ratio(aspect_ratio);
    _input = string_from_ratio(aspect_ratio);
}

auto string_from_ratio(float ratio) -> std::string
{
    if (std::abs(ratio - 1.41421356f) < 0.001f)
        return "A4 Horizontal";
    if (std::abs(ratio - 0.70710678f) < 0.001f)
        return "A4 Vertical";
    smart::Fraction const fraction = smart::as_fraction(ratio);

    bool const fraction_is_small_enough =
        std::abs(fraction.numerator) <= 30
        && std::abs(fraction.denominator) <= 30;

    return fraction_is_small_enough
               ? stringify(fraction)
               : fmt::format("{:.3f}", ratio);
}

auto AspectRatio::imgui(float width, const char* label) -> bool
{
    bool b = false;

    static const auto ratios = std::array{
        // Sorted from biggest to smallest
        std::make_pair("     16 / 9     ", 16.f / 9.f),
        std::make_pair("      3 / 2     ", 3.f / 2.f),
        std::make_pair("  A4 Horizontal ", std::sqrt(2.f)),
        std::make_pair("      4 / 3     ", 4.f / 3.f),
        std::make_pair("      1 / 1     ", 1.f),
        std::make_pair("      3 / 4     ", 3.f / 4.f),
        std::make_pair("   A4 Vertical  ", 1.f / std::sqrt(2.f)),
        std::make_pair("      2 / 3     ", 2.f / 3.f),
        std::make_pair("      9 / 16    ", 9.f / 16.f),
    };

    ImGui::SetNextItemWidth(
        width != 0.f
            ? width
            : ImGuiExtras::calc_custom_dropdown_input_width()
    );

    if (ImGui::InputText("##aspect_ratio_input", &_input, ImGuiInputTextFlags_AutoSelectAll, nullptr, nullptr, ImDrawFlags_RoundCornersLeft))
    {
        _ratio = parse_ratio(_input);
        b      = true;
    }
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        _input = string_from_ratio(_ratio);
    }
    bool const input_text_is_hovered = ImGui::IsItemHovered();

    ImGui::SameLine(0.f, 0.f);

    if (ImGui::BeginCombo(label, "", ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft, ImDrawFlags_RoundCornersRight))
    {
        for (auto const& ratio : ratios)
        {
            if (ImGui::Selectable(ratio.first, false))
            {
                set(ratio.second);
                b = true;
            }
        }
        ImGui::EndCombo();
    }

    if ((input_text_is_hovered || ImGui::IsItemHovered()) && ImGui::GetIO().MouseWheel != 0.f)
    {
        bool const find_bigger = ImGui::GetIO().MouseWheel > 0.f;
        int const  sz          = static_cast<int>(ratios.size());
        for (int i = find_bigger ? sz - 1 : 0; find_bigger ? (i >= 0) : (i < sz); i += find_bigger ? -1 : 1)
        {
            float const ratio = ratios.at(static_cast<size_t>(i)).second;
            if (find_bigger ? (ratio > _ratio) : (ratio < _ratio))
            {
                set(ratio);
                b = true;
                break;
            }
        }
    }

    return b;
}

} // namespace Cool

#if COOL_ENABLE_TESTS
#include <doctest/doctest.h>
TEST_CASE("parse_ratio()")
{
    CHECK(doctest::Approx{Cool::parse_ratio("")} == 1.f);
    CHECK(doctest::Approx{Cool::parse_ratio("/")} == 1.f);
    CHECK(doctest::Approx{Cool::parse_ratio(".")} == 1.f);
    CHECK(doctest::Approx{Cool::parse_ratio("3")} == 3.f);
    CHECK(doctest::Approx{Cool::parse_ratio("3.")} == 3.f);
    CHECK(doctest::Approx{Cool::parse_ratio("3./")} == 3.f);
    CHECK(doctest::Approx{Cool::parse_ratio("3/")} == 3.f);
    CHECK(doctest::Approx{Cool::parse_ratio("3/.")} == 3.f);
    CHECK(doctest::Approx{Cool::parse_ratio("3/.5")} == 6.f);
    CHECK(doctest::Approx{Cool::parse_ratio("3/2")} == 1.5f);
    CHECK(doctest::Approx{Cool::parse_ratio("3./2")} == 1.5f);
    CHECK(doctest::Approx{Cool::parse_ratio("3./2.")} == 1.5f);
    CHECK(doctest::Approx{Cool::parse_ratio("3/2.")} == 1.5f);
    CHECK(doctest::Approx{Cool::parse_ratio("0.5")} == 0.5f);
    CHECK(doctest::Approx{Cool::parse_ratio(".5")} == 0.5f);
    CHECK(doctest::Approx{Cool::parse_ratio("/2")} == 0.5f);
    CHECK(doctest::Approx{Cool::parse_ratio("/2.")} == 0.5f);
}
#endif