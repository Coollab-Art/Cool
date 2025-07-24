#pragma once
#include "Cool/Gpu/Texture.h"
#include "Cool/Path/Path.h"

namespace Cool {

struct TextureSource_Image {
    std::filesystem::path absolute_path{Cool::Path::default_texture()};

    auto               imgui_widget() -> bool;
    [[nodiscard]] auto get_texture() const -> Texture const*;
    [[nodiscard]] auto get_error() const -> std::optional<std::string>;

    friend auto operator==(TextureSource_Image const&, TextureSource_Image const&) -> bool = default;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            ser20::make_nvp("Path", absolute_path)
        );
    }
    template<class Archive>
    void load(Archive& archive)
    {
        archive(
            ser20::make_nvp("Path", absolute_path)
        );
        if (absolute_path.empty())
            absolute_path = Cool::Path::default_texture();
    }
};

} // namespace Cool