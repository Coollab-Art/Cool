#pragma once
#include <img/src/Size.h>
#include <cstdint>
#include "Cool/ColorSpaces/AlphaSpace.h"
#include "Cool/WebGPU/TextureView.h"
#include "Cool/WebGPU/WGPUUnique.h"
#include "webgpu/webgpu.hpp"

namespace Cool {

class Texture : public WGPUUnique<wgpu::Texture> {
public:
    Texture() = default; // TODO(WebGPU) Remove ?
    explicit Texture(wgpu::TextureDescriptor const&);
    void set_image(uint32_t color_components_count, bool need_to_flip_y, AlphaSpace, void const* data, size_t data_size, size_t data_element_size); // TODO(WebGPU) give default value of false to need_to_flip_y? Put it in a struct with AlphaSpace and other config?

    void save(std::filesystem::path const&) const;
    /// NB: the callback will be called asynchronously, several frames later // TODO(WebGPU) for now this isn't the case, we block until it is finished. Should we do async or not ?
    void with_pixels(std::function<void(std::span<uint8_t const>)> const& callback) const;

    auto               imgui_texture_id() const -> ImTextureID { return entire_texture_view().handle(); }
    [[nodiscard]] auto size() const -> img::Size { return img::Size{width(), height()}; }
    [[nodiscard]] auto width() const -> uint32_t { return _desc.size.width; }
    [[nodiscard]] auto height() const -> uint32_t { return _desc.size.height; }
    [[nodiscard]] auto aspect_ratio() const -> float { return img::aspect_ratio(size()); }
    [[nodiscard]] auto alpha_space() const -> AlphaSpace { return _alpha_space; }
    [[nodiscard]] auto need_to_flip_y() const -> bool { return _need_to_flip_y; }

    [[nodiscard]] auto ref() const -> TextureRef { return {id(), size()}; }
    auto               entire_texture_view() const -> TextureView const&; // TODO(WebGPU) When resizing the texture, we probably need to rebuild the view ?
    auto               descriptor() const -> wgpu::TextureDescriptor { return _desc; }

private:
    friend class RenderTarget;
    void set_alpha_space(AlphaSpace alpha_space) { _alpha_space = alpha_space; }

private:
    // TODO(ColorSpaces) Store the color space of the texture (Oklab, sRGB, etc.) and use it to check / know if we need conversions etc.
    wgpu::TextureDescriptor            _desc{};
    AlphaSpace                         _alpha_space{};
    bool                               _need_to_flip_y{false};
    mutable std::optional<TextureView> _entire_texture_view{};
};

auto load_texture(std::filesystem::path const& path, wgpu::TextureFormat texture_format, bool need_to_flip_y, std::optional<AlphaSpace> = {}) -> tl::expected<Texture, std::string>;
auto texture_from_pixels(img::Size size, wgpu::TextureFormat texture_format, bool need_to_flip_y, AlphaSpace, void const* data, size_t data_size, size_t data_element_size) -> Texture;
auto texture_1D_from_data(uint32_t width, wgpu::TextureFormat texture_format, size_t channels_count, void const* data, size_t data_size, size_t data_element_size) -> Texture;

template<typename Container>
auto texture_from_pixels(img::Size size, wgpu::TextureFormat texture_format, bool need_to_flip_y, AlphaSpace alpha_space, Container const& data) -> Texture
{
    return texture_from_pixels(size, texture_format, alpha_space, reinterpret_cast<void const*>(data.data()), data.size(), sizeof(Container::value_type)); // NOLINT(*reinterpret-cast)
}
template<typename Container>
auto texture_1D_from_data(uint32_t width, wgpu::TextureFormat texture_format, size_t channels_count, Container const& data) -> Texture
{
    return texture_1D_from_data(width, texture_format, channels_count, reinterpret_cast<void const*>(data.data()), data.size(), sizeof(Container::value_type)); // NOLINT(*reinterpret-cast)
}

} // namespace Cool