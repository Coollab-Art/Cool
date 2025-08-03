#pragma once
#include <SpoutGL/SpoutSenderNames.h>
#include "Cool/Gpu/Texture.h"
#include "SpoutGL/SpoutReceiver.h"

namespace Cool {

// TODO(Spout) Disable on linux and mac (the source is responsible for the error message)

struct SpoutData {
    SpoutReceiver receiver{};
    Cool::Texture sharedTexture{};
};

class TextureLibrary_Spout {
public:
    [[nodiscard]] auto get_texture(std::string const& sender_name) -> Texture const*;
    [[nodiscard]] auto get_error(std::string const& sender_name) const -> std::optional<std::string>;

    auto get_sender_names() -> std::vector<std::string>;

    void shut_down(); // TODO(Spout) call this somewhere

private:
    std::list<SpoutData> _spouts{}; // We use a list instead of a vector because Spout objects are not movable (more precisely: their move constructor is buggy and will cause a crash, we could take the time to fix the library)
    spoutSenderNames     _names{};
};

inline auto texture_library_spout() -> TextureLibrary_Spout&
{
    static auto instance = TextureLibrary_Spout{};
    return instance;
}

} // namespace Cool