#pragma once
#include "AspectRatio.h"

namespace Cool {

struct SharedAspectRatio {
    AspectRatio aspect_ratio{};
    bool        fill_the_view{true};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Aspect Ratio", aspect_ratio),
            ser20::make_nvp("Fill the View", fill_the_view)
        );
    }
};

} // namespace Cool