#pragma once

namespace Cool {

struct TextureRef {
    GLuint    id{};
    img::Size size{};

    auto width() const { return size.width(); }
    auto height() const { return size.height(); }

    [[nodiscard]] auto imgui_texture_id() const -> ImTextureID { return static_cast<ImTextureID>(id); }

    auto download_pixels() const -> img::Image;
};

} // namespace Cool