#pragma once

namespace Cool {

struct TextureRef {
    GLuint    id{}; // TODO(WebGPU)
    img::Size size{};

    auto width() const { return size.width(); }
    auto height() const { return size.height(); }

    [[nodiscard]] auto imgui_texture_id() const -> ImTextureID { return reinterpret_cast<ImTextureID>(static_cast<uint64_t>(id)); }

    auto download_pixels() const -> img::Image;
};

} // namespace Cool