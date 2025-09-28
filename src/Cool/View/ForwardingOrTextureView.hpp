#pragma once
#include "TextureView.hpp"

namespace Cool {

class ForwardingOrTextureView : public TextureView {
public:
    ForwardingOrTextureView(View const& forwarded_view, ViewCreationParams const& p)
        : TextureView{p}
        , _forwarded_view{forwarded_view}
    {}

private:
    auto get_image_texture_id() const -> ImTextureID override { return _forwarded_view.is_open()
                                                                           ? _forwarded_view.get_image_texture_id()
                                                                           : TextureView::get_image_texture_id(); }
    auto get_image_size() const -> img::Size override { return _forwarded_view.is_open()
                                                                   ? _forwarded_view.get_image_size()
                                                                   : TextureView::get_image_size(); }

private:
    View const& _forwarded_view; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

} // namespace Cool
