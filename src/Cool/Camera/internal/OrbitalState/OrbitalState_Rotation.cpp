#include "OrbitalState_Rotation.h"
#include <Cool/Constants/Constants.h>
#include <smart/smart.hpp>
#include "../../Camera.h"
#include "../../ViewController_Orbital.h"
#include "OrbitalState_Idle.h"

namespace Cool {

OrbitalState_Rotation::OrbitalState_Rotation(const Camera& camera)
    : _sign_of_alignment_with_up{smart::sign(glm::dot(camera.up_axis(), Constants::world_up))}
{
}

auto OrbitalState_Rotation::on_drag(ViewController_Orbital& controller, Camera& camera, ImGuiCoordinates const& delta) -> bool
{
    const auto orbit_center = controller.get_orbit_center(camera);
    const auto up_axis      = [&]() {
        switch (controller._mode)
        {
        case ViewController_Orbital::Mode::Trackball:
            return Constants::world_up;
        case ViewController_Orbital::Mode::Turntable:
        {
            return Constants::world_up * _sign_of_alignment_with_up;
        }
        case ViewController_Orbital::Mode::AxisFree:
            return camera.up_axis();
        }
        assert(false);
        return glm::vec3{0};
    }();
    camera.translate(-orbit_center);
    camera.rotate(-controller._rotation_speed * delta.x, up_axis);
    camera.rotate(-controller._rotation_speed * delta.y, camera.right_axis());
    camera.translate(+orbit_center);
    return true;
}

auto OrbitalState_Rotation::on_drag_stop(ViewController_Orbital& controller, Camera&) -> bool
{
    controller.set_state(OrbitalState_Idle{});
    return false;
}

} // namespace Cool