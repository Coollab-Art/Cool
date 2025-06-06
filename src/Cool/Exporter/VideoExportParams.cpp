#include "VideoExportParams.h"
#include <imgui.h>
#include <cstdint>
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "internal/ORIGIN_OF_FRAMES.hpp"

namespace Cool {

static auto time_to_frames(Time time, double fps) -> int64_t
{
    return static_cast<int64_t>(std::ceil(time.as_seconds_double() * fps)) + ORIGIN_OF_FRAMES;
}

static auto frames_to_time(int64_t frame, double fps) -> Time
{
    return Time::seconds(static_cast<double>(frame - ORIGIN_OF_FRAMES) / fps);
}

void VideoExportParams::imgui()
{
    ImGui::Text("Framerate");
    ImGui::SameLine();
    ImGui::InputScalar("##FPS", ImGuiDataType_Double, &fps, nullptr, nullptr, "%.2f fps");

    static bool input_as_time_instead_of_frames{true}; // A static is what we want here, this is a global option that the user can choose and that should be shared across all widgets.
    if (input_as_time_instead_of_frames)
    {
        ImGui::PushItemWidth(0.4f * (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Fromto").x));
        ImGui::Text("From");
        ImGui::SameLine();
        imgui_drag_time("##beginning", &beginning);
        ImGui::SameLine();
        ImGui::Text("to");
        ImGui::SameLine();
        imgui_drag_time("##end", &end);
        ImGui::PopItemWidth();
    }
    else
    {
        ImGui::PushItemWidth(ImGui::CalcTextSize(std::to_string(ORIGIN_OF_FRAMES).c_str()).x + 2.f * ImGui::GetStyle().FramePadding.x);
        ImGui::Text("From");
        ImGui::SameLine();
        {
            int64_t beginning_frame = time_to_frames(beginning, fps);
            if (ImGui::InputScalar("##beginning2", ImGuiDataType_S64, &beginning_frame))
                beginning = frames_to_time(beginning_frame, fps);
        }
        ImGui::SameLine();
        ImGui::Text("to");
        ImGui::SameLine();
        {
            int64_t end_frame = time_to_frames(end, fps);
            if (ImGui::InputScalar("##end2", ImGuiDataType_S64, &end_frame))
                end = frames_to_time(end_frame, fps);
        }
        ImGui::SameLine();
        ImGui::Text("frames");
        ImGui::PopItemWidth();
    }
    ImGui::SameLine();
    if (ImGuiExtras::button_with_text_icon(ICOMOON_STACK))
        input_as_time_instead_of_frames = !input_as_time_instead_of_frames;
    ImGui::SetItemTooltip("Switch units (Time or Frames)");
}

} // namespace Cool