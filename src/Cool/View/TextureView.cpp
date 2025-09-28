#include "TextureView.hpp"

namespace Cool {

auto TextureView::desired_image_size(ImageSizeConstraint const& constraint) const -> img::Size
{
    if (!window_size().has_value())
        return {};
    return constraint.applied_to(*window_size());
}

auto TextureView::get_image_texture_id() const -> ImTextureID
{
    return _texture.imgui_texture_id();
}
auto TextureView::get_image_size() const -> img::Size
{
    return _texture.size;
}

} // namespace Cool