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

auto TextureLibrary_Spout::get_texture(std::string const& sender_name) -> Texture const*
{
    auto it = std::find_if(_spouts.begin(), _spouts.end(), [&](SpoutData& data) {
        return data.receiver.GetSenderName() == sender_name;
    });
    if (it == _spouts.end())
    {
        _spouts.emplace_back();
        it = std::prev(_spouts.end());
        _spouts.back().receiver.SetVerticalSync(false); // TODO(Spout) This seems to reduce lag, but is it a good idea?
    }
    auto& receiver      = it->receiver;
    auto& sharedTexture = it->sharedTexture;

    receiver.SetReceiverName(sender_name.c_str());

    // if (!webcam_data.has_been_requested_this_frame)
    // {
    //     webcam_data.has_been_requested_this_frame = true;
    //     webcam_data.maybe_image                   = webcam_data.webcam.image(); // We need to keep the image alive till the end of the frame, so we take a copy of the shared_ptr. The image stored in the webcam_data.webcam can be destroyed at any time if a new image is created by the background thread
    // }
    GLint currentFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

    unsigned int w = sharedTexture.size().width();
    unsigned int h = sharedTexture.size().width();

    char bob[256];

    if (receiver.ReceiveTexture(bob /*senderName*/, w, h, sharedTexture.id(), GL_TEXTURE_2D, true /*inverted*/, currentFBO))
    {
        sharedTexture.bob_size(w, h);
        // Texture received successfully
        // Use sharedTextureID for rendering
        // Cool::Log::info("spout", senderName);
        // std::cout << senderName << ' ' << w << ' ' << h << std::endl;
        // needs_to_rerender_flag().set_dirty(); // TODO(Spout) warn dependency
    }
    if (receiver.IsUpdated())
    {
        sharedTexture.set_size({receiver.GetSenderWidth(), receiver.GetSenderHeight()});
    }
    return &sharedTexture;
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