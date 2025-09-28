#include "ViewController_Orbital.h"
#include "Camera.h"
#include "CameraU.h"

namespace Cool {

ViewController_Orbital::ViewController_Orbital(ViewController_Orbital::Mode mode)
    : _mode{mode}
{
}

bool ViewController_Orbital::ImGui(Camera& camera)
{
    const auto prev_mode = _mode;

    int current_combo_item = [&]() {
        switch (_mode)
        {
        case Mode::Turntable:
            return 0;
        case Mode::Trackball:
            return 1;
        case Mode::AxisFree:
            return 2;
        }
        assert(false);
        return 0;
    }();
    auto const b = ImGui::Combo("Orbit Mode", &current_combo_item, " Turntable\0 Trackball\0 AxisFree\0\0");
    if (b)
    {
        switch (current_combo_item)
        {
        case 0:
            _mode = Mode::Turntable;
            break;
        case 1:
            _mode = Mode::Trackball;
            break;
        case 2:
            _mode = Mode::AxisFree;
            break;
        default:
            assert(false);
            break;
        }
        if (prev_mode == Mode::AxisFree && _mode != Mode::AxisFree)
        {
            CameraU::reset_roll(camera);
        }
    }
    return b;
}

void ViewController_Orbital::set_orbit_center(glm::vec3 const& orbit_center, Camera& camera) const
{
    camera.translate(orbit_center - get_orbit_center(camera));
}

glm::vec3 ViewController_Orbital::get_orbit_center(Camera const& camera) const
{
    return camera.position() + _distance_to_orbit_center * camera.front_axis();
}

} // namespace Cool