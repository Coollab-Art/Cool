#include "ImageSizeConstraint.h"
#include <Cool/ImGui/ImGuiExtras.h>
#include "compute_image_size.h"

namespace Cool {

auto ImageSizeConstraint::applied_to(img::Size frame_size) const -> img::Size
{
    if (!does_constrain_size())
        return frame_size;

    float const aspect_ratio = _shared_aspect_ratio->fill_the_view
                                   ? img::aspect_ratio(frame_size)
                                   : _shared_aspect_ratio->aspect_ratio.get();

    auto const nb_pixels = static_cast<float>(
        _is_controlling_nb_pixels
            ? _nb_pixels
            : frame_size.width() * frame_size.height()
    );

    return compute_image_size(aspect_ratio, nb_pixels);
}

auto ImageSizeConstraint::imgui_aspect_ratio() -> bool // NOLINT(*make-member-function-const)
{
    bool b = false;
    b |= ImGuiExtras::toggle("Fill the View", &_shared_aspect_ratio->fill_the_view);
    ImGuiExtras::disabled_if(_shared_aspect_ratio->fill_the_view, "Unselect \"Fill the View\" above if you want to control the aspect ratio manually", [&]() {
        b |= _shared_aspect_ratio->aspect_ratio.imgui();
    });
    return b;
}

auto ImageSizeConstraint::imgui_nb_pixels() -> bool
{
    return ImGuiExtras::toggle_with_submenu("Limit render size", &_is_controlling_nb_pixels, [&]() {
        return ImGuiExtras::slider_uint32("Number of Pixels", &_nb_pixels, 10'000, 500'000);
    });
}

} // namespace Cool