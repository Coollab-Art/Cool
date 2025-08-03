#if defined(COOL_SPOUT)
#include "TextureLibrary_Spout.hpp"

namespace Cool {

// TODO(Spout) Dependencies: when we use a Spout node, rerender when the given Spout receives a new texture? Or simply every frame, which is more than likely what will happen anyways

// void TextureLibrary_Spout::on_frame_end()
// {
//     std::erase_if(_spouts, [&](WebcamData const& data) {
//         return !data.has_been_requested_this_frame;
//     });
//     for (auto& data : _spouts)
//         data.has_been_requested_this_frame = false;
// }

// TODO(Spout) fallback to purple texture when sender disconnects ? or if it has never been connected

// TODO(Spout) destroy sender + texture when unused for a while

auto TextureLibrary_Spout::get_sender_names() -> std::vector<std::string>
{
    std::set<std::string> sendernames;
    _names_getter.GetSenderNames(&sendernames);
    return {sendernames.begin(), sendernames.end()};
}

auto TextureLibrary_Spout::get_texture(std::string const& sender_name) -> Texture const*
{
    auto it = std::find_if(_spouts.begin(), _spouts.end(), [&](SpoutData& data) {
        return data.receiver.GetSenderName() == sender_name;
    });
    if (it == _spouts.end())
    {
        _spouts.emplace_back();
        it = std::prev(_spouts.end());
        _spouts.back().receiver.SetVerticalSync(false); // Otherwise there is some noticeable lag in the video
    }
    auto& receiver = it->receiver;
    auto& texture  = it->texture;

    receiver.SetReceiverName(sender_name.c_str());

    // if (!webcam_data.has_been_requested_this_frame)
    // {
    //     webcam_data.has_been_requested_this_frame = true;
    //     webcam_data.maybe_image                   = webcam_data.webcam.image(); // We need to keep the image alive till the end of the frame, so we take a copy of the shared_ptr. The image stored in the webcam_data.webcam can be destroyed at any time if a new image is created by the background thread
    // }
    GLint currentFBO; // NOLINT(*init-variables)
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

    receiver.ReceiveTexture(texture.id(), GL_TEXTURE_2D, true /*inverted*/, currentFBO);

    if (receiver.IsUpdated())
        texture.set_size({receiver.GetSenderWidth(), receiver.GetSenderHeight()});
    return &texture;
}

auto TextureLibrary_Spout::get_error(std::string const& sender_name) const -> std::optional<std::string>
{
    return {};
    // auto it = std::find_if(_spouts.begin(), _spouts.end(), [&](WebcamData const& data) {
    //     return data.webcam.id() == id;
    // });
    // if (it == _spouts.end())
    //     return std::nullopt;

    // auto const* error = std::get_if<wcam::CaptureError>(&it->maybe_image);
    // if (!error)
    //     return std::nullopt;
    // return wcam::to_string(*error);
}

void TextureLibrary_Spout::shut_down()
{
    _spouts.clear();
}

} // namespace Cool

#endif