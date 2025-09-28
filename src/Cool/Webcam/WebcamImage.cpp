#include "WebcamImage.hpp"
#include <wcam/src/FirstRowIs.hpp>
#include "Cool/Gpu/Texture.h"

namespace Cool {

class TexturePool { // Pool to avoid to recreate the textures all the time, which costs a lost. Just setting the data each time is much cheaper
public:
    auto take(img::Size size) -> Texture
    {
        auto it = std::find_if(_textures.begin(), _textures.end(), [&](Texture const& texture) {
            return texture.size() == size;
        });
        if (it == _textures.end())
        {
            _textures.emplace_back();
            auto texture = std::move(_textures.back());
            _textures.pop_back();
            return texture;
        }
        else // NOLINT(*else-after-return)
        {
            auto texture = std::move(*it);
            _textures.erase(it);
            return texture;
        }
    }

    void give_back(Texture texture)
    {
        _textures.push_back(std::move(texture));
    }

private:
    std::list<Texture> _textures{};
};

auto texture_pool() -> TexturePool&
{
    static auto instance = TexturePool{};
    return instance;
}

WebcamImage::~WebcamImage()
{
    if (_texture)
        texture_pool().give_back(std::move(*_texture));
}

auto WebcamImage::get_texture() const -> Texture const&
{
    if (_gen_texture.has_value())
    {
        (*_gen_texture)();
        _gen_texture.reset();
    }
    return *_texture;
}

void WebcamImage::set_data(wcam::ImageDataView<wcam::RGB24> const& rgb_data)
{
    _gen_texture = [owned_rgb_data = rgb_data.to_owning(), this]() {
        auto const size = img::Size{
            owned_rgb_data.resolution().width(),
            owned_rgb_data.resolution().height()
        };
        auto const layout = TextureLayout{
            .internal_format = InternalFormat::RGBA,
            .channels        = Channels::RGB,
            .texel_data_type = TexelDataType::UnsignedByte,
        };
        auto const* data = owned_rgb_data.data();

        if (!_texture)
            _texture = texture_pool().take(size); // Take a texture of the right size, so that we won't have to resize it, which saves perfs
        _texture->set_image(size, data, layout, owned_rgb_data.row_order() == wcam::FirstRowIs::Top);
    };
}

void WebcamImage::set_data(wcam::ImageDataView<wcam::BGR24> const& bgr_data)
{
    _gen_texture = [owned_bgr_data = bgr_data.to_owning(), this]() {
        auto const size = img::Size{
            owned_bgr_data.resolution().width(),
            owned_bgr_data.resolution().height()
        };
        auto const layout = TextureLayout{
            .internal_format = InternalFormat::RGBA,
            .channels        = Channels::BGR,
            .texel_data_type = TexelDataType::UnsignedByte,
        };
        auto const* data = owned_bgr_data.data();

        if (!_texture)
            _texture = texture_pool().take(size); // Take a texture of the right size, so that we won't have to resize it, which saves perfs
        _texture->set_image(size, data, layout, owned_bgr_data.row_order() == wcam::FirstRowIs::Top);
    };
}

} // namespace Cool