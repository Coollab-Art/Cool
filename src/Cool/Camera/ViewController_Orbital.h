#pragma once
#include "Cool/Input/MouseCoordinates.h"
#include "internal/OrbitalState/OrbitalState.h"

namespace Cool {

class Camera;

class ViewController_Orbital {
public:
    enum class Mode {
        Trackball,
        Turntable,
        AxisFree,
    };

public:
    ViewController_Orbital(ViewController_Orbital::Mode mode = ViewController_Orbital::Mode::Turntable);

    bool ImGui(Camera& camera);

    glm::vec3 get_orbit_center(Camera const& camera) const;

    void        set_orbit_center(glm::vec3 const& orbit_center, Camera& camera) const;
    inline void set_distance_to_orbit_center(float distance) { _distance_to_orbit_center = distance; }
    inline auto get_distance_to_orbit_center() const -> float { return _distance_to_orbit_center; }
    inline void set_state(OrbitalState state) { _state = state; }

    // clang-format off
    /// Returns true iff the camera values have changed
    inline auto on_drag        (Camera& camera, ImGuiCoordinates const& delta) -> bool{ return std::visit([&](auto&& state) { return state.on_drag         (*this, camera, delta); }, _state); }
	/// Returns true iff the camera values have changed
    inline auto on_drag_start  (Camera& camera)                                -> bool{ return std::visit([&](auto&& state) { return state.on_drag_start   (*this, camera);  }, _state); }
	/// Returns true iff the camera values have changed
    inline auto on_drag_stop   (Camera& camera)                                -> bool{ return std::visit([&](auto&& state) { return state.on_drag_stop    (*this, camera);        }, _state); }
	/// Returns true iff the camera values have changed
    inline auto on_wheel_scroll(Camera& camera, float dl)                      -> bool{ return std::visit([&](auto&& state) { return state.on_wheel_scroll (*this, camera, dl);    }, _state); }
    // clang-format on

private:
    friend class OrbitalState_Idle;
    friend class OrbitalState_Rotation;
    friend class OrbitalState_Translation;

private:
    ViewController_Orbital::Mode _mode;
    float                        _translation_speed        = 0.01f;
    float                        _rotation_speed           = 0.01f;
    float                        _distance_to_orbit_center = 5.f;
    OrbitalState                 _state                    = OrbitalState_Idle{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Mode", _mode),
            ser20::make_nvp("Translation Speed", _translation_speed),
            ser20::make_nvp("Rotation Speed", _rotation_speed),
            ser20::make_nvp("Distance to Orbit Center", _distance_to_orbit_center)
        );
    }
};

} // namespace Cool