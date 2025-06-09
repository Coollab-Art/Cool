#pragma once
#include <imgui_gradient/imgui_gradient.hpp>
#include <ser20/types/list.hpp>
#include "Cool/Serialization/ImGui.hpp"

namespace Cool {
struct Gradient {
    ImGG::GradientWidget value{};
    ImGG::WrapMode       wrap_mode{ImGG::WrapMode::MirrorRepeat};

    friend auto operator==(const Gradient& a, const Gradient& b) -> bool = default;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void save(Archive& archive) const
    {
        const auto wrap_mode_size_t          = static_cast<size_t>(wrap_mode);
        const auto interpolation_mode_size_t = static_cast<size_t>(value.gradient().interpolation_mode());
        archive(
            ser20::make_nvp("Gradient", value.gradient().get_marks()),
            ser20::make_nvp("Wrap Mode", wrap_mode_size_t),
            ser20::make_nvp("Interpolation Mode", interpolation_mode_size_t)
        );
    }
    friend class ser20::access;
    template<class Archive>
    void load(Archive& archive)
    {
        std::list<ImGG::Mark> gradient_loaded{};
        size_t                wrap_mode_size_t{};
        size_t                interpolation_mode_size_t{};
        archive(gradient_loaded, wrap_mode_size_t, interpolation_mode_size_t);
        value.gradient()                      = ImGG::Gradient{gradient_loaded};
        wrap_mode                             = static_cast<ImGG::WrapMode>(wrap_mode_size_t);
        value.gradient().interpolation_mode() = static_cast<ImGG::Interpolation>(interpolation_mode_size_t);
    }
};

auto to_string(Cool::Gradient const&) -> std::string;

auto imgui_widget(std::string_view name, Cool::Gradient& gradient, bool should_use_a_random_color_for_the_new_marks, ImGuiColorEditFlags flags) -> bool;

namespace internal {
auto gradient_marks_array_name(std::string_view name) -> std::string;
} // namespace internal

} // namespace Cool

namespace ser20 {

template<class Archive>
void serialize(Archive& archive, ImGG::Mark& mark)
{
    archive(
        ser20::make_nvp("Mark position", mark.position),
        ser20::make_nvp("Mark color", mark.color)
    );
}

template<class Archive>
void save(Archive& archive, ImGG::RelativePosition const& position)
{
    archive(ser20::make_nvp("Relative position", position.get()));
}

template<class Archive>
void load(Archive& archive, ImGG::RelativePosition& position)
{
    float position_loaded{};
    archive(position_loaded);
    position = ImGG::RelativePosition{position_loaded};
}

} // namespace ser20