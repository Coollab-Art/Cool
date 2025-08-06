#pragma once
#if defined(COOL_SPOUT)
#include "Cool/Gpu/Texture.h"
#include "SpoutGL/SpoutReceiver.h"
#include "SpoutGL/SpoutSenderNames.h"

namespace Cool {

class TextureLibrary_Spout {
public:
    auto get_texture(std::string const& sender_name) -> Texture const*;
    auto get_error(std::string const& sender_name) const -> std::optional<std::string>;

    auto get_sender_names() const -> std::vector<std::string>;

    void imgui_debug_view() const;
    void on_frame_end();
    void shut_down();

private:
    auto sender_is_connected(std::string const& sender_name) const -> bool;

private:
    struct SpoutData {
        mutable SpoutReceiver receiver{};
        Cool::Texture         texture{};
        bool                  has_been_requested_this_frame{false};
    };

    std::list<SpoutData>     _spouts{}; // We use a list instead of a vector because Spout objects are not movable (more precisely: their move constructor is buggy and will cause a crash, we could take the time to fix the library)
    mutable spoutSenderNames _names_getter{};
};

inline auto texture_library_spout() -> TextureLibrary_Spout&
{
    static auto instance = TextureLibrary_Spout{};
    return instance;
}

} // namespace Cool

#endif