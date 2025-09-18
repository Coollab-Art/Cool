#include "TextureLibrary_Video.h"
#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>
#include "Cool/Gpu/Texture.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Time/time_formatted_hms.h"
#include "Cool/Video/hack_get_global_time_in_seconds.h"
#include "imgui.h"

namespace Cool {

static constexpr auto time_to_live = 1min;

static auto time_to_live_has_expired(std::chrono::steady_clock::time_point date_of_last_request) -> bool
{
    return std::chrono::steady_clock::now() - date_of_last_request > time_to_live;
}

auto TextureLibrary_Video::get(VideoRequest const& request) -> Texture const*
{
    auto const key = Key{request.id, request.video_descriptor.path};
    auto       it  = _videos.find(key);
    if (it == _videos.end())
    {
        it = _videos.insert({key, Cool::TextureLibrary_Video::Data{
                                      .player{Cool::VideoPlayer{request.video_descriptor}},
                                      .date_of_last_request = clock::now(),
                                  }})
                 .first;
    }
    else
    {
        it->second.player.settings() = request.video_descriptor.settings; // Make sure to update the settings
    }

    // We have a new request
    it->second.date_of_last_request = clock::now();

    return it->second.player.get_texture(hack_get_global_time_in_seconds());
}

void TextureLibrary_Video::update()
{
    std::erase_if(_videos, [&](auto const& kv) {
        return time_to_live_has_expired(kv.second.date_of_last_request);
    });
}

auto TextureLibrary_Video::error_from(VideoRequest const& request) -> std::optional<std::string>
{
    auto it = _videos.find({request.id, request.video_descriptor.path});
    if (it == _videos.end())
        return std::nullopt;
    return it->second.player.get_error();
}

auto TextureLibrary_Video::detailed_video_info(std::filesystem::path const& path) const -> std::string const*
{
    for (auto const& kv : _videos)
    {
        if (kv.first.path != path)
            continue;

        std::string const* res = kv.second.player.detailed_video_info();
        if (res == nullptr)
            continue;

        return res;
    }
    return nullptr;
}

void TextureLibrary_Video::imgui_debug_view() const
{
    for (auto const& kv : _videos)
    {
        {
            auto const* maybe_tex = kv.second.player.get_current_texture();
            if (maybe_tex)
                ImGuiExtras::image_framed(maybe_tex->imgui_texture_id(), {100.f * maybe_tex->aspect_ratio(), 100.f}, {.frame_thickness = 2.f, .flip_y = maybe_tex->need_to_flip_y()});
            else
                ImGui::TextUnformatted("INVALID");
        }
        ImGui::SameLine();
        ImGui::BeginGroup(); // So that the two texts are on the right of the image
        {
            ImGui::TextUnformatted(kv.first.path.string().c_str());
        }
        {
            auto const time_since_last_use = std::chrono::steady_clock::now() - kv.second.date_of_last_request;
            if (time_since_last_use < time_to_live)
            {
                auto const duration = time_to_live - time_since_last_use;
                ImGui::TextUnformatted(time_formatted_hms(duration).c_str());
            }
            else
            {
                ImGui::TextUnformatted(fmt::format("Expired (Not used for {})", time_to_live).c_str());
            }
        }
        ImGui::EndGroup();
    }
}

} // namespace Cool