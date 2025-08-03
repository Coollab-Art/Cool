#pragma once
#include "Cool/Gpu/Texture.h"
#include "SpoutGL/SpoutReceiver.h"

namespace Cool {

// TODO(Spout) Disable on linux and mac (the source is responsible for the error message)

struct SpoutData {
    SpoutReceiver receiver;
    Cool::Texture sharedTexture{img::Size{1280, 720}};
};

class TextureLibrary_Spout {
public:
    [[nodiscard]] auto get_texture(std::string const& sender_name) -> Texture const*;
    [[nodiscard]] auto get_error(std::string const& sender_name) const -> std::optional<std::string>;

    void shut_down(); // TODO(Spout) call this somewhere

private:
    std::vector<SpoutData> _spouts{};
};

inline auto texture_library_spout() -> TextureLibrary_Spout&
{
    static auto instance = TextureLibrary_Spout{};
    return instance;
}

} // namespace Cool