#pragma once
#include <optional>
#include <string>
#include <wcam/wcam.hpp>
#include "Cool/WebGPU/Texture.h"
#include "Cool/Serialization/wcam_serialization.h"

namespace Cool {

class TextureSource_Webcam {
public:
    auto               imgui_widget() -> bool;
    [[nodiscard]] auto get_texture() const -> Texture const*;
    [[nodiscard]] auto get_error() const -> std::optional<std::string>;

    friend auto operator==(TextureSource_Webcam const& a, TextureSource_Webcam const& b) -> bool
    {
        return a._device_id == b._device_id;
    }

private:
    void set_device_id(wcam::DeviceId const& id) const;

private:
    mutable wcam::DeviceId _device_id{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Device ID", _device_id)
        );
    }
};

} // namespace Cool