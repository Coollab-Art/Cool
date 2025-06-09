#pragma once
#include <imgui/imgui.h>

namespace ser20 {

template<class Archive>
void serialize(Archive& archive, ImVec2& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y)
    );
}

template<class Archive>
void serialize(Archive& archive, ImVec4& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y),
        ser20::make_nvp("z", v.z),
        ser20::make_nvp("w", v.w)
    );
}

} // namespace ser20