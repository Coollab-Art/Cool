#pragma once
#include <Cool/Image/ImageSizeU.h>
#include <Cool/Image/SharedAspectRatio.hpp>
#include <img/img.hpp>

namespace Cool {

class ExportSize {
public:
    operator img::Size() const { return _size; }

    auto imgui() -> bool;

    void set_shared_aspect_ratio(SharedAspectRatio& shared_aspect_ratio) { _shared_aspect_ratio = &shared_aspect_ratio; }

private:
    void apply_aspect_ratio();

private:
    img::Size          _size{1920, 1080};
    bool               _aspect_ratio_is_locked{true};
    SharedAspectRatio* _shared_aspect_ratio{};
    ImageSizeU::WH     _last_changed_side{ImageSizeU::WH::None};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Size", _size),
            ser20::make_nvp("Aspect Ratio is Locked", _aspect_ratio_is_locked)
        );
    }
};

} // namespace Cool