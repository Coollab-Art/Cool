#pragma once

namespace Cool {

enum class PixelType : std::uint8_t {
    UInt8,
    Float16,
    Float32,
};

struct TextureFormat {
    int       num_components{4};
    PixelType type{PixelType::UInt8};
};

} // namespace Cool