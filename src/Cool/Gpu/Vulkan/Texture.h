#pragma once
#if defined(COOL_VULKAN)

#include <img/img.hpp>

namespace Cool::Vulkan {

class Texture {
public:
    struct Id;
    Texture() = default;
    Texture(
        img::Size              size,
        vk::Format             format                           = vk::Format::eR8G8B8A8Unorm,
        vk::ImageLayout        layout_when_read_by_imgui_shader = vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageUsageFlagBits additional_usage_flags           = (vk::ImageUsageFlagBits)0
    );
    Texture(
        const img::Image&      image_data,
        vk::Format             format                           = vk::Format::eR8G8B8A8Unorm,
        vk::ImageLayout        layout_when_read_by_imgui_shader = vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageUsageFlagBits additional_usage_flags           = (vk::ImageUsageFlagBits)0
    );
    Texture(std::filesystem::path path);

    /**
     * @brief Recreates the underlying image with a new size. Warning : the data is lost and the image is reset to black !
     *
     * @param size
     */
    void resize(img::Size size);

    Id                         id();
    ImTextureID                imgui_texture_id() const { return _imgui_texture_id; }
    const vku::TextureImage2D& image() const { return _vku; }
    vku::TextureImage2D&       image() { return _vku; }
    int                        nb_channels() const { return 4; } // TODO(JF) fix me
    img::Image                 download_pixels() const;

    struct Id {
        operator ImTextureID() const
        {
            return static_cast<ImTextureID>(id);
        }
#if defined(COOL_OPENGL)
        GLuint id;
#elif defined(COOL_VULKAN)
        VkImage id;
#endif
    };

private:
    mutable vku::TextureImage2D _vku;
    vk::UniqueSampler           _sampler;
    vk::ImageLayout             _layout_when_read_by_imgui_shader;
    ImTextureID                 _imgui_texture_id;
};

} // namespace Cool::Vulkan

#endif