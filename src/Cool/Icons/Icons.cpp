#include "Icons.h"
#include <Cool/DebugOptions/DebugOptions.h>
#include <Cool/Log/ToUser.h>
#include "Cool/WebGPU/Texture.h"

namespace Cool {

std::map<std::filesystem::path, Texture> Icons::_map;

const Texture& Icons::get(std::filesystem::path image_path)
{
    const auto path = File::make_absolute(image_path);
    auto       res  = _map.find(path);
    if (res == _map.end())
    {
        {
            auto texture = load_texture(path, wgpu::TextureFormat::RGBA8Unorm);
            if (texture.has_value())
                _map[path] = std::move(*texture);
            else
                // TODO(WebGPU) log the error in the texture which is a tl::expected
                _map[path] = Texture{dummy_image()}; // TODO(WebGPU) instead of creating a dummy texture for each missing icon, can't we reuse the same one? This doesn't matter though because no icons should be missing
        }
        if (DebugOptions::log_when_creating_textures())
            Log::info("Icons", fmt::format("Generated texture from \"{}\"", Cool::File::weakly_canonical(path)));
        return _map[path];
    }
    else
    {
        return res->second;
    }
}

void Icons::cleanup_texture(std::filesystem::path const& image_path)
{
    const auto path = File::make_absolute(image_path);
    auto       res  = _map.find(path);
    if (res == _map.end())
    {
        Log::internal_warning("Icons::cleanup_texture", fmt::format("The texture you want to clean up doesn't exist! \"{}\"", Cool::File::weakly_canonical(path)));
        return;
    }
    _map.erase(path);
}

} // namespace Cool