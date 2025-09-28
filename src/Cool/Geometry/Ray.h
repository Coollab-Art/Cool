#pragma once

namespace Cool {

/// Holds an origin and a normalized direction
class Ray {
public:
    /// Direction doesn't need to be normalized because the constructor will do it anyways
    Ray(glm::vec3 const& origin, glm::vec3 const& direction)
        : _origin(origin), _direction(glm::normalize(direction))
    {
    }

    glm::vec3 const& origin() { return _origin; }
    glm::vec3 const& direction() { return _direction; }

private:
    glm::vec3 _origin;
    glm::vec3 _direction;
};

} // namespace Cool