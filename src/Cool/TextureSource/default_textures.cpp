#include "default_textures.h"

namespace Cool {

auto dummy_image() -> img::Image const& // TODO(WebGPU) do we still need this dummy_image?
{
    static auto instance = img::Image{img::Size{1, 1}, img::PixelFormat::RGB, img::FirstRowIs::Bottom, new uint8_t[]{255, 0, 255}};
    return instance;
}

auto dummy_texture() -> Texture const&
{
    static auto instance = texture_from_pixels(dummy_image().size(), wgpu::TextureFormat::RGBA8Unorm, AlphaSpace::Any, dummy_image().data_span()); // TODO(WebGPU) check that it still works, we used to pass an array with 4 values, but now the dummy_image() only has 3 components (no alpha)
    return instance;
}

auto transparent_texture() -> Texture const&
{
    static auto instance = texture_from_pixels(img::Size{1, 1}, wgpu::TextureFormat::RGBA8Unorm, AlphaSpace::Any, std::array<uint8_t, 4>{0, 0, 0, 0});
    return instance;
}

} // namespace Cool
