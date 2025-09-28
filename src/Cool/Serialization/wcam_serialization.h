#pragma once
#include <wcam/src/DeviceId.hpp>
#include <wcam/src/Resolution.hpp>

namespace ser20 {

template<class Archive>
void save(Archive& archive, wcam::DeviceId const& id)
{
    archive(ser20::make_nvp("ID", id.as_string()));
}

template<class Archive>
void load(Archive& archive, wcam::DeviceId& id)
{
    std::string s;
    archive(s);
    id = wcam::DeviceId::from_string(s);
}

template<class Archive>
void save(Archive& archive, wcam::Resolution const& resolution)
{
    archive(
        ser20::make_nvp("Width", resolution.width()),
        ser20::make_nvp("Height", resolution.height())
    );
}

template<class Archive>
void load(Archive& archive, wcam::Resolution& resolution)
{
    wcam::Resolution::DataType w, h; // NOLINT(*readability-isolate-declaration, *init-variables)
    archive(w, h);
    resolution = {w, h};
}

} // namespace ser20