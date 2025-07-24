#pragma once
#include "Cool/Gpu/Texture.h"
#include "Cool/Video/VideoDescriptor.h"

namespace Cool {

class TextureSource_Video {
public:
    auto               imgui_widget() -> bool;
    [[nodiscard]] auto get_texture() const -> Texture const*;
    [[nodiscard]] auto get_error() const -> std::optional<std::string>;

    friend auto operator==(TextureSource_Video const&, TextureSource_Video const&) -> bool = default;

private:
    static uint32_t next_id;
    VideoDescriptor _video_descriptor{};
    uint32_t        _id{next_id++};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Video", _video_descriptor)
        );
    }
};

} // namespace Cool