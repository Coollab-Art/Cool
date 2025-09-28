#pragma once
#include <img/img.hpp>
#include "Cool/ColorSpaces/AlphaSpace.h"
#include "Cool/Path/Path.h"
#include "Cool/WebGPU/ComputePipeline.h"
#include "Cool/WebGPU/Texture.h"
#include "RenderTargetInfo.h"

namespace Cool {

// TODO(WebGPU) Some render targets will render in straight alpha directly (e.g. checkerboard pattern), and don't need the conversion from premultiplied to straight
// We should probably store a AlphaSpace on the Texture (users give it to us when they call render(), and we pass it to the texture)
class RenderTarget {
public:
    /// With `alpha_space` you must indicate the space that your render function will output
    void render(AlphaSpace alpha_space, std::function<void(wgpu::RenderPassEncoder render_pass)> const& render_fn);
    // img::Image  download_pixels() const { return img::load(Cool::Path::root() / "res/images/logo.png"); /*texture_straight_alpha().download_pixels();*/ } // Saved images are expected to be straight alpha

    img::Image  download_pixels() const { return _texture.download_pixels(); }
    auto        texture_ref() const -> TextureRef { return _texture.ref(); }
    ImTextureID imgui_texture_id() const { return texture_straight_alpha().imgui_texture_id(); } // ImGui expects straight alpha
    img::Size   current_size() const { return _texture.size(); }
    img::Size   desired_size() const { return _desired_size; }
    void        set_size(img::Size size);
    void        set_size_immediately(img::Size size);
    auto        needs_resizing() const -> bool;

    auto texture_straight_alpha() const -> Texture const&;
    auto texture_premultiplied_alpha() const -> Texture const&;

private:
    void resize_if_necessary();
    void make_texture_in(AlphaSpace) const;
    auto texture_in(AlphaSpace) const -> Texture const&;

private:
    Texture                        _texture{};
    mutable std::optional<Texture> _texture_in_other_alpha_space{};
    img::Size                      _desired_size{};
};

} // namespace Cool