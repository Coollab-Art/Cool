#pragma once
#include <img/img.hpp>
#include "../RenderTargetInfo.h"
#include "../TextureFormat.hpp"
#include "Cool/Gpu/OpenGL/TextureRef.hpp"

namespace Cool {

template<typename RenderTarget_Impl>
class RenderTarget_Base {
public:
    explicit RenderTarget_Base(TextureFormat format)
        : _impl{format}
    {}

    void             render(typename RenderTarget_Impl::RenderFuncType render_fn);
    img::Image       download_pixels() const { return _impl.download_pixels(); }
    ImTextureID      imgui_texture_id() const { return _impl.imgui_texture_id(); }
    auto             texture_ref() const -> TextureRef { return _impl.texture_ref(); }
    RenderTargetInfo info() const { return _impl.info(); }
    img::Size        current_size() const { return _impl.size(); }
    img::Size        desired_size() const { return _desired_size; }
    void             set_size(img::Size size) { _desired_size = size; }
    void             set_size_immediately(img::Size size)
    {
        set_size(size);
        resize_if_necessary();
    }
    RenderTarget_Impl&       get() { return _impl; }       // TODO(JF) This is a temporary solution until the abstraction is fully done
    RenderTarget_Impl const& get() const { return _impl; } // TODO(JF) This is a temporary solution until the abstraction is fully done
    auto                     needs_resizing() const -> bool { return _impl.size() != desired_size(); }

private:
    void resize_if_necessary();

private:
    RenderTarget_Impl _impl;
    img::Size         _desired_size;
};

} // namespace Cool

#include "RenderTarget_Base.tpp"
