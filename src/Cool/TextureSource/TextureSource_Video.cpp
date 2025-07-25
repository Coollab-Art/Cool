#include "TextureSource_Video.h"
#include <open/open.hpp>
#include "Cool/File/File.h"
#include "TextureLibrary_Video.h"

namespace Cool {

uint32_t TextureSource_Video::next_id{0};

auto TextureSource_Video::imgui_widget() -> bool
{
    return _video_descriptor.imgui_widget();
}

[[nodiscard]] auto TextureSource_Video::get_texture() const -> Texture const*
{
    return TextureLibrary_Video::instance().get({_id, _video_descriptor});
}

auto TextureSource_Video::get_error_notification() const -> std::optional<ImGuiNotify::Notification>
{
    auto const err = TextureLibrary_Video::instance().error_from({_id, _video_descriptor});
    if (!err.has_value())
        return std::nullopt;

    return ImGuiNotify::Notification{
        .type                 = ImGuiNotify::Type::Error,
        .title                = "Video error",
        .content              = fmt::format("\"{}\"\n\n{}", Cool::File::weakly_canonical(_video_descriptor.path), *err),
        .custom_imgui_content = [path = _video_descriptor.path]() {
            if (ImGui::Button("Try to open file in explorer"))
                Cool::open_focused_in_explorer(path);
        },
        .duration = std::nullopt,
        .closable = false,
    };
}

} // namespace Cool