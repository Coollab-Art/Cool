#pragma once
#include <Cool/Gpu/RenderTarget.h>
#include <Cool/Image/ImageSizeConstraint.h>
#include "View.h"

namespace Cool {

/// A View that displays a given texture.
class TextureView : public View {
public:
    using View::View;

    void set_texture(TextureRef texture) { _texture = texture; }
    auto desired_image_size(ImageSizeConstraint const&) const -> img::Size;

protected:
    auto get_image_texture_id() const -> ImTextureID override;
    auto get_image_size() const -> img::Size override;

private:
    TextureRef _texture{};
};

} // namespace Cool
