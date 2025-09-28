#include "TextureLibrary_Image.hpp"
#include <chrono>
#include <filesystem>
#include <optional>
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/File/File.h"
#include "Cool/FileWatcher/FileWatcher.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/TextureSource/default_textures.h"
#include "Cool/Time/time_formatted_hms.h"
#include "Cool/WebGPU/Texture.h"
#include "fmt/chrono.h"
#include "imgui.h"

namespace Cool {

auto TextureLibrary_Image::get(std::filesystem::path const& path, std::chrono::milliseconds time_to_live) -> Texture const*
{
    auto const it = _textures.find(path);
    // If this path is not known to the library, add it and try again.
    if (it == _textures.end())
    {
        _textures[path] = {
            .file_watcher         = FileWatcher{path, FileWatcher_NoCallbacks()},
            .date_of_last_request = clock::now(),
            .time_to_live         = time_to_live,
        };
        reload_texture(path);
        return get(path, time_to_live);
    }

    // We have a new request
    it->second.date_of_last_request = clock::now();
    it->second.time_to_live         = std::max(it->second.time_to_live, time_to_live);

    auto const& maybe_tex = it->second.texture;
    if (!maybe_tex && !it->second.error_message) // Texture is nullopt simply because it hasn't been used in a while. Reload the texture.
        reload_texture(path);

    if (!maybe_tex)
        return nullptr;
    return &*maybe_tex;
}

void TextureLibrary_Image::reload_texture(std::filesystem::path const& path)
{
    auto& tex         = _textures[path];
    tex.error_message = std::nullopt;
    tex.texture       = std::nullopt;

    auto texture = load_texture(path, wgpu::TextureFormat::RGBA8Unorm);
    if (texture.has_value())
    {
        tex.texture = std::move(*texture);
        if (DebugOptions::log_when_creating_textures())
            Log::info("TextureLibrary_Image", fmt::format("Generated texture from \"{}\"", Cool::File::weakly_canonical(path)));
    }
    else
    {
        tex.error_message = std::move(texture.error());
    }
}

auto TextureLibrary_Image::update() -> bool
{
    bool       has_changed = false;
    auto const reload_tex  = [&](std::filesystem::path const& path) {
        reload_texture(path);
        has_changed = true;
    };
    for (auto& kv : _textures)
    {
        kv.second.file_watcher.update({.on_file_changed = reload_tex, .on_path_invalid = reload_tex});
        if (std::chrono::steady_clock::now() - kv.second.date_of_last_request > kv.second.time_to_live)
            kv.second.texture = std::nullopt;
    }

    return has_changed;
}

auto TextureLibrary_Image::error_from(std::filesystem::path const& path) -> std::optional<std::string>
{
    return _textures[path].error_message;
}

void TextureLibrary_Image::imgui_debug_view() const
{
    for (auto const& kv : _textures)
    {
        ImGuiExtras::image_framed(
            kv.second.texture ? kv.second.texture->imgui_texture_id() : dummy_texture().imgui_texture_id(),
            {100.f * (kv.second.texture ? kv.second.texture->aspect_ratio() : 1.f), 100.f},
            {.frame_thickness = 2.f}
        );

        ImGui::SameLine();
        ImGui::BeginGroup();

        ImGui::TextUnformatted(kv.first.string().c_str());

        {
            auto const time_since_last_use = std::chrono::steady_clock::now() - kv.second.date_of_last_request;
            auto const time_to_live        = kv.second.time_to_live;
            if (time_since_last_use < time_to_live)
                ImGui::TextUnformatted(time_formatted_hms(time_to_live - time_since_last_use).c_str());
            else
                ImGui::TextUnformatted(fmt::format("Expired (Not used for {})", time_to_live).c_str());
        }

        ImGui::EndGroup();
    }
}

} // namespace Cool