#pragma once
#include <Cool/Path/Path.h>
#include <map>
#include "Cool/WebGPU/Texture.h"

namespace Cool {

class Icons {
public:
    /**
     * @brief
     *
     * @return The texture ID of the "close_button" icon
     */
    static const Texture& close_button() { return get(Path::cool_res() / "icons/close_button.png"); }

    /**
     * @brief Returns the OpenGL texture containing the image. The first time get() is called for a given image path, the file is read and a texture is generated. Subsequent calls to get() for this same file will serve the texture from a cache (i.e. really fast). If you want to force textures to be preloaded, you can call get("myImage.png") at the beginning of your app. /!\ Once get() is called and a texture is generated, it remains in GPU memory until the end of the application. If you don't need a texture anymore, you can use cleanup_texture().
     *
     * @param image_path The path to the image file. Make sure you use an absolute path (e.g. Cool::Path::root() / "myPath/relativeTo/theExecutablesDirectory.png") to prevent problems that would arise when changing the working directory during the app's lifetime
     * @return The OpenGL texture ID of the image. The first time get is called for a given image path, the file is read and a texture is generated. Subsequent calls to get for this same file will serve the texture ID from a cache (i.e. really fast). If you want to force textures to be preloaded, you can call get("myImage.png") at the beginning of your app.
     */
    static const Texture& get(std::filesystem::path image_path);

    /**
     * @brief Destroys the OpenGL texture. It is recommended to only use this if you know you won't use that texture for a while and it is taking too much place in memory
     *
     * @param image_path The path to the image file. Make sure you use an absolute path (e.g. Cool::Path::root() / "myPath/relativeTo/theExecutablesDirectory.png") to prevent problems that would arise when changing the working directory during the app's lifetime
     */
    static void cleanup_texture(std::filesystem::path const& image_path);

private:
    static std::map<std::filesystem::path, Texture> _map;
};

} // namespace Cool