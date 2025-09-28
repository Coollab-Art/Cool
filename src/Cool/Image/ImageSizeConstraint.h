#pragma once
#include <Cool/WebGPU/InterpolationMode.hpp>
#include <img/img.hpp>
#include "SharedAspectRatio.hpp"

namespace Cool {

class ImageSizeConstraint {
public:
    auto does_constrain_size() const -> bool { return does_constrain_aspect_ratio() || _is_controlling_nb_pixels; }
    auto does_constrain_aspect_ratio() const -> bool { return !_shared_aspect_ratio->fill_the_view; }
    auto applied_to(img::Size frame_size) const -> img::Size;

    auto imgui_aspect_ratio() -> bool;
    auto imgui_nb_pixels() -> bool;

    void set_shared_aspect_ratio(SharedAspectRatio& shared_aspect_ratio) { _shared_aspect_ratio = &shared_aspect_ratio; }

private:
    bool               _is_controlling_nb_pixels = false;
    uint32_t           _nb_pixels                = 250'000;
    SharedAspectRatio* _shared_aspect_ratio{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Is controlling the number of pixels", _is_controlling_nb_pixels),
            ser20::make_nvp("Number of pixels", _nb_pixels)
        );
    }
};

} // namespace Cool