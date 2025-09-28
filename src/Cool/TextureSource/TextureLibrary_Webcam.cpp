#include "TextureLibrary_Webcam.hpp"
#include <wcam/wcam.hpp>
#include "Cool/Webcam/WebcamImage.hpp"

namespace Cool {

auto TextureLibrary_Webcam::instance() -> TextureLibrary_Webcam&
{
    static auto inst = TextureLibrary_Webcam{};
    return inst;
}

void TextureLibrary_Webcam::on_frame_end()
{
    std::erase_if(_webcams, [&](WebcamData const& data) {
        return !data.has_been_requested_this_frame;
    });
    for (auto& data : _webcams)
        data.has_been_requested_this_frame = false;
}

auto TextureLibrary_Webcam::get_texture(wcam::DeviceId const& id) -> Texture const*
{
    auto it = std::find_if(_webcams.begin(), _webcams.end(), [&](WebcamData const& data) {
        return data.webcam.id() == id;
    });
    if (it == _webcams.end())
    {
        _webcams.emplace_back(wcam::open_webcam(id));
        it = std::prev(_webcams.end());
    }
    auto& webcam_data = *it;
    if (!webcam_data.has_been_requested_this_frame)
    {
        webcam_data.has_been_requested_this_frame = true;
        webcam_data.maybe_image                   = webcam_data.webcam.image(); // We need to keep the image alive till the end of the frame, so we take a copy of the shared_ptr. The image stored in the webcam_data.webcam can be destroyed at any time if a new image is created by the background thread
    }
    auto const* image = std::get_if<std::shared_ptr<wcam::Image const>>(&webcam_data.maybe_image);
    if (!image)
        return nullptr;
    return &static_cast<WebcamImage const*>(image->get())->get_texture(); // NOLINT(*static-cast-downcast)
}

auto TextureLibrary_Webcam::get_error(wcam::DeviceId const& id) const -> std::optional<std::string>
{
    auto it = std::find_if(_webcams.begin(), _webcams.end(), [&](WebcamData const& data) {
        return data.webcam.id() == id;
    });
    if (it == _webcams.end())
        return std::nullopt;

    auto const* error = std::get_if<wcam::CaptureError>(&it->maybe_image);
    if (!error)
        return std::nullopt;
    return wcam::to_string(*error);
}

void TextureLibrary_Webcam::shut_down()
{
    _webcams.clear();
}

} // namespace Cool