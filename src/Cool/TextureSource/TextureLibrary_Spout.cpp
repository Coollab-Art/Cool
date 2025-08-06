#if defined(COOL_SPOUT)
#include "TextureLibrary_Spout.hpp"
#include "Cool/ImGui/ImGuiExtras.h"

namespace Cool {

void TextureLibrary_Spout::on_frame_end()
{
    std::erase_if(_spouts, [&](SpoutData& data) { return !data.has_been_requested_this_frame; });
    for (auto& data : _spouts)
        data.has_been_requested_this_frame = false;
}

auto TextureLibrary_Spout::get_sender_names() const -> std::vector<std::string>
{
    auto names = std::set<std::string>{};
    _names_getter.GetSenderNames(&names);
    return std::vector<std::string>{names.begin(), names.end()};
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

    auto const receive_texture = [&]() { receiver.ReceiveTexture(texture.id(), GL_TEXTURE_2D, true /*inverted*/, static_cast<GLuint>(currentFBO)); };

    receive_texture();
    if (receiver.IsUpdated()) // e.g. the texture size has changed
    {
        if (texture.set_size_ifn({receiver.GetSenderWidth(), receiver.GetSenderHeight()}))
            // Texture has been resized and therefore pixels have been cleared so we need to receive it again to avoid having an empty texture for 1 frame
            receive_texture();
    }
    return &texture;
}

auto TextureLibrary_Spout::get_error(std::string const& sender_name) const -> std::optional<std::string>
{
    if (!sender_is_connected(sender_name))
        return fmt::format("This Spout sender is not connected", sender_name);

    return std::nullopt;
}

void TextureLibrary_Spout::shut_down()
{
    _spouts.clear();
}

void TextureLibrary_Spout::imgui_debug_view() const
{
    for (auto const& data : _spouts)
    {
        ImGuiExtras::image_framed(
            data.texture.imgui_texture_id(),
            {100.f * data.texture.aspect_ratio(), 100.f},
            {.frame_thickness = 2.f, .flip_y = data.texture.need_to_flip_y()}
        );
        ImGui::SameLine();
        ImGui::TextUnformatted(data.receiver.GetSenderName());
    }
}

} // namespace Cool

#endif