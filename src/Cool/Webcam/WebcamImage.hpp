#pragma once
#include "Cool/Gpu/Texture.h"
#include "wcam/wcam.hpp"

namespace Cool {

class WebcamImage : public wcam::Image { // NOLINT(*special-member-functions)
public:
    ~WebcamImage() override;

    auto get_texture() const -> Texture const&;

    void set_data(wcam::ImageDataView<wcam::RGB24> const& rgb_data) override;
    void set_data(wcam::ImageDataView<wcam::BGR24> const& bgr_data) override;

private:
    mutable std::optional<Texture>               _texture{};
    mutable std::optional<std::function<void()>> _gen_texture{}; // TODO(WebGPU) Since OpenGL calls must happen on the main thread, when set_data is called (from another thread) we just store the thing to do in this function, and call it later, on the main thread
};

} // namespace Cool