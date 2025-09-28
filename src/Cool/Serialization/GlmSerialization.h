#pragma once

namespace ser20 {

template<class Archive>
void serialize(Archive& archive, glm::vec2& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::vec3& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y),
        ser20::make_nvp("z", v.z)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::vec4& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y),
        ser20::make_nvp("z", v.z),
        ser20::make_nvp("w", v.w)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::uvec2& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::uvec3& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y),
        ser20::make_nvp("z", v.z)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::uvec4& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y),
        ser20::make_nvp("z", v.z),
        ser20::make_nvp("w", v.w)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::ivec2& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::ivec3& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y),
        ser20::make_nvp("z", v.z)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::ivec4& v)
{
    archive(
        ser20::make_nvp("x", v.x),
        ser20::make_nvp("y", v.y),
        ser20::make_nvp("z", v.z),
        ser20::make_nvp("w", v.w)
    );
}

template<class Archive>
void serialize(Archive& archive, glm::mat2& m)
{
    archive(
        m[0][0], m[0][1],
        m[1][0], m[1][1]
    );
}

template<class Archive>
void serialize(Archive& archive, glm::mat3& m)
{
    archive(
        m[0][0], m[0][1], m[0][2],
        m[1][0], m[1][1], m[1][2],
        m[2][0], m[2][1], m[2][2]
    );
}

template<class Archive>
void serialize(Archive& archive, glm::mat4& m)
{
    archive(
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]
    );
}

} // namespace ser20