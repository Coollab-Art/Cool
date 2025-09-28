#include "ExportSize.h"
#include <Cool/ImGui/ImGuiExtras.h>
#include <imgui.h>
#include <smart/smart.hpp>
#include <stringify/stringify.hpp>
#include "Cool/ImGui/IcoMoonCodepoints.h"

namespace Cool {

void ExportSize::apply_aspect_ratio()
{
    // Fun fact : our algorithm doesn't match https://www.adobe.com/uk/creativecloud/design/discover/a4-format.html
    // For example, for an A4 ratio the website suggests 3508x2480
    // but for a height of 2480, a width  of 3507 gives us a better approximation of the A4 ratio than 3508
    // and for a width  of 3508, a height of 2481 gives us a better approximation of the A4 ratio than 2480
    // So the A4 formats listed on the website are worse approximations than what this algorithm will give you.
    if (_last_changed_side == ImageSizeU::WH::Width)
    {
        _size.set_height(static_cast<decltype(_size)::DataType>(
            std::round(static_cast<float>(_size.width()) / _shared_aspect_ratio->aspect_ratio.get())
        ));
    }
    else
    {
        _size.set_width(static_cast<decltype(_size)::DataType>(
            std::round(static_cast<float>(_size.height()) * _shared_aspect_ratio->aspect_ratio.get())
        ));
    }
}

auto ExportSize::imgui() -> bool
{
    if (ImGui::IsWindowAppearing() || _aspect_ratio_is_locked)
        apply_aspect_ratio();
    _shared_aspect_ratio->fill_the_view = false; // Fixed aspect ratio as soon as we open the export window. This avoids the case where we open the export window, then resize the main window which makes the View change aspect ratio. In that case we don't want the current aspect ratio and size that we see in the export window to change. And to preserve the consistency between the aspect ratio in the exporter window and in the View, we have to keep the aspect ratio fixed.

    bool b = false;

    auto const changed_side = ImageSizeU::imgui(_size);
    if (changed_side != ImageSizeU::WH::None)
    {
        _last_changed_side = changed_side; // Only update if changed_side != ImageSizeU::WH::None because we want to remember the last side that actually changed.
        if (!_aspect_ratio_is_locked)
            _shared_aspect_ratio->aspect_ratio.set(img::aspect_ratio(_size));

        b = true;
    }

    ImGui::SameLine();
    ImGuiExtras::disabled_if(!_aspect_ratio_is_locked, "Ratio is not locked, you cannot edit it directly. Lock it with the button on the right.", [&]() {
        b |= _shared_aspect_ratio->aspect_ratio.imgui(150.f);
    });
    ImGui::SameLine();
    b |= ImGuiExtras::checkbox_button(ICOMOON_LINK, &_aspect_ratio_is_locked);
    ImGui::SetItemTooltip(
        "%s",
        _aspect_ratio_is_locked
            ? "The aspect ratio is locked. Changing the width will automatically change the height (and vice versa) to keep the same ratio."
            : "The aspect ratio is unlocked. The width and the height can be changed independently."
    );

    return b;
}

} // namespace Cool