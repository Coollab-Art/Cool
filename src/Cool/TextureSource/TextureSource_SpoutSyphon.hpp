#pragma once
#include "Cool/Gpu/Texture.h"

namespace Cool {

class TextureSource_SpoutSyphon {
public:
    auto               imgui_widget() -> bool;
    [[nodiscard]] auto get_texture() const -> Texture const*;
    [[nodiscard]] auto get_error() const -> std::optional<std::string>;

    friend auto operator==(TextureSource_SpoutSyphon const& a, TextureSource_SpoutSyphon const& b) -> bool = default;

private:
    std::string _sender_name{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Sender Name", _sender_name)
        );
    }
};

} // namespace Cool