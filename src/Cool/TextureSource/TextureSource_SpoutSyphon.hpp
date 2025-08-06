#pragma once
#include <ImGuiNotify/ImGuiNotify.hpp>
#include "Cool/Gpu/Texture.h"

namespace Cool {

class TextureSource_SpoutSyphon {
public:
    auto               imgui_widget() -> bool;
    [[nodiscard]] auto get_texture() const -> Texture const*;
    [[nodiscard]] auto get_error_notification() const -> std::optional<ImGuiNotify::Notification>;

    friend auto operator==(TextureSource_SpoutSyphon const& a, TextureSource_SpoutSyphon const& b) -> bool = default;

private:
    void init_sender_name_ifn() const;

private:
    mutable std::string _sender_name{};

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