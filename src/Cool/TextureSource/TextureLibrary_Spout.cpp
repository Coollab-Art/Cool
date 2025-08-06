#include <SpoutGL/SpoutUtils.h>
#if defined(COOL_SPOUT)
#include "TextureLibrary_Spout.hpp"

namespace Cool {

void TextureLibrary_Spout::on_frame_end()
{
    auto const names = get_sender_names();
    std::erase_if(_spouts, [&](SpoutData& data) {
        return !data.has_been_requested_this_frame
               || !sender_is_connected(data.receiver.GetSenderName()); // TODO(Spout) also, when we get_texture, need to check that list
    });
    for (auto& data : _spouts)
        data.has_been_requested_this_frame = false;
}

// TODO(Spout) fallback to purple texture when sender disconnects ? or if it has never been connected

// TODO(Spout) destroy sender + texture when unused for a while

auto TextureLibrary_Spout::get_sender_names() const -> std::vector<std::string>
{
    std::set<std::string> sendernames;
    _names_getter.GetSenderNames(&sendernames);
    return {sendernames.begin(), sendernames.end()};
}

auto TextureLibrary_Spout::sender_is_connected(std::string const& sender_name) const -> bool
{
    auto const names = get_sender_names();
    return std::find(names.begin(), names.end(), sender_name) != names.end();
}

auto TextureLibrary_Spout::get_texture(std::string const& sender_name) -> Texture const*
{
    if (!sender_is_connected(sender_name))
        return nullptr;

    auto it = std::find_if(_spouts.begin(), _spouts.end(), [&](SpoutData& data) {
        return data.receiver.GetSenderName() == sender_name;
    });
    if (it == _spouts.end())
    {
        _spouts.emplace_back();
        it = std::prev(_spouts.end());
        _spouts.back().receiver.SetReceiverName(sender_name.c_str());
        _spouts.back().receiver.SetVerticalSync(false); // Otherwise there is some noticeable lag in the video
    }
    auto& receiver                    = it->receiver;
    auto& texture                     = it->texture;
    it->has_been_requested_this_frame = true;

    GLint currentFBO; // NOLINT(*init-variables)
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

    receiver.ReceiveTexture(texture.id(), GL_TEXTURE_2D, true /*inverted*/, currentFBO);
    if (receiver.IsUpdated())
    {
        if (texture.set_size_ifn({receiver.GetSenderWidth(), receiver.GetSenderHeight()}))
            // Texture has been resized and therefore pixels have been cleared so we need to receive it again to avoid having an empty texture for 1 frame
            receiver.ReceiveTexture(texture.id(), GL_TEXTURE_2D, true /*inverted*/, currentFBO);
    }
    return &texture;
}

// TODO(Spout) debug option to view all the textures in the lib (append us to the thing that already has image and video, and add webcam as well)

auto TextureLibrary_Spout::get_error(std::string const& sender_name) const -> std::optional<std::string>
{
    if (!sender_is_connected(sender_name))
        return fmt::format("The spout sender has been disconnected", sender_name);

    return std::nullopt;
}

void TextureLibrary_Spout::shut_down()
{
    _spouts.clear();
}

} // namespace Cool

#endif