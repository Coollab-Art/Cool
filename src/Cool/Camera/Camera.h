#pragma once

#include <Cool/Constants/Constants.h>
#include "Projection_Perspective.h"

namespace Cool {

class Camera {
public:
    Camera(glm::vec3 const& position = Constants::default_camera_position, glm::vec3 const& look_at = glm::vec3{0.f});

    auto      transform_matrix() const -> glm::mat4 { return _transform_matrix; }
    auto      view_matrix() const -> glm::mat4 { return glm::inverse(_transform_matrix); }
    auto      projection_matrix(float aspect_ratio) const -> glm::mat4 { return Cool::projection_matrix(_projection, aspect_ratio); }
    auto      inverse_projection_matrix(float aspect_ratio) const -> glm::mat4 { return glm::inverse(projection_matrix(aspect_ratio)); }
    auto      inverse_view_projection_matrix(float aspect_ratio) const -> glm::mat4;
    auto      view_projection_matrix(float aspect_ratio) const -> glm::mat4;
    glm::vec3 right_axis() const;
    glm::vec3 up_axis() const;
    glm::vec3 front_axis() const;
    glm::vec3 position() const;
    float     far_plane() const { return _projection.far_plane; }
    float&    far_plane() { return _projection.far_plane; }

    auto projection() -> Projection_Perspective& { return _projection; }
    auto projection() const -> const Projection_Perspective& { return _projection; }

    inline void set_transform_matrix(const glm::mat4& transform_matrix) { _transform_matrix = transform_matrix; }
    inline void set_view_matrix(const glm::mat4& view_matrix) { _transform_matrix = glm::inverse(view_matrix); }

    /// Translation expressed in world space
    void translate(glm::vec3 const& delta_position);

    /**
     * @brief Rotation expressed in world space
     *
     * @param center
     * @param angle
     * @param axis
     */
    void rotate_around(glm::vec3 const& center, float angle, glm::vec3 const& axis);
    void rotate(float angle, glm::vec3 const& axis);

    auto operator==(const Camera&) const -> bool = default;

private:
    glm::mat4              _transform_matrix = glm::mat4{1.f};
    Projection_Perspective _projection{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Transform", _transform_matrix),
            ser20::make_nvp("Projection", _projection)
        );
    }
};

} // namespace Cool