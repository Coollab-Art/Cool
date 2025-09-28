#include "markdown.h"
#include <imgui_markdown/imgui_markdown.h>
#include <open/open.hpp>
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/ImGuiExtrasStyle.h"
#include "Fonts.h"
#include "imgui.h"

namespace Cool::ImGuiExtras {

static void link_clicked_callback(ImGui::MarkdownLinkCallbackData data)
{
    if (data.isImage)
        return;
    open_link(std::string{data.link, static_cast<size_t>(data.linkLength)}.c_str()); // `link` is not a zero-terminated string, so we must construct a string from the pointer and the length.
}

static void format_callback(ImGui::MarkdownFormatInfo const& info, bool is_beginning);

void markdown(std::string_view markdown_text)
{
    static auto const config = ImGui::MarkdownConfig{
        .linkCallback   = &link_clicked_callback,
        .formatCallback = &format_callback,
    };
    ImGui::Markdown(markdown_text.data(), markdown_text.length(), config);
}

static void format_emphasis(ImGui::MarkdownFormatInfo const& info, bool is_beginning)
{
    auto const is_italic = info.level == 1;
    if (is_beginning)
        ImGui::PushFont(is_italic ? Font::italic() : Font::bold());
    else
        ImGui::PopFont();
}

static void format_heading(ImGui::MarkdownFormatInfo const& info, bool is_beginning)
{
    auto* const font = info.level == 1
                           ? Font::heading_1()
                       : info.level == 2
                           ? Font::heading_2()
                       : info.level == 3
                           ? Font::heading_3()
                           : Font::bold();
    if (is_beginning)
    {
        ImGui::PushFont(font);
    }
    else
    {
        ImGui::PopFont();
        if (info.level <= 2)
            ImGui::Separator();
    }
}

static void format_link(ImGui::MarkdownFormatInfo const& info, bool is_beginning)
{
    auto const color = info.itemHovered ? ImGuiExtras::GetStyle().url_link_hovered : ImGuiExtras::GetStyle().url_link;
    if (is_beginning)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
    }
    else
    {
        ImGui::PopStyleColor();
        ImGui::UnderLine(color);
        if (info.itemHovered)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
    }
}

static void format_callback(ImGui::MarkdownFormatInfo const& info, bool is_beginning)
{
    switch (info.type)
    {
    case ImGui::MarkdownFormatType::NORMAL_TEXT:
        break;
    case ImGui::MarkdownFormatType::EMPHASIS:
        format_emphasis(info, is_beginning);
        break;
    case ImGui::MarkdownFormatType::HEADING:
        format_heading(info, is_beginning);
        break;
    case ImGui::MarkdownFormatType::UNORDERED_LIST:
        break;
    case ImGui::MarkdownFormatType::LINK:
        format_link(info, is_beginning);
        break;
    }
}

} // namespace Cool::ImGuiExtras