#include "test_markdown_formatting.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/markdown.h"

namespace Cool {

void test_markdown_formatting()
{
    static auto text = std::string{R"markdown(
some text

# Heading 1
some text

## Heading 2
some text

### Heading 3
some text

*emphasis*
**strong emphasis**

Normal text
  Indent level 1
    Indent level 2
      Indent level 3
Normal text

Normal text
  * Unordered List level 1
    * Unordered List level 2
      * Unordered List level 3
      * Unordered List level 3
  * Unordered List level 1
Normal text

[Link](https://coollab-art.com)

![image](image identifier e.g. filename)

Separator:
___

)markdown"};

    auto const child_window_width = 0.5f * ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
    ImGui::BeginChild("##1", ImVec2(child_window_width, 0.f));
    ImGuiExtras::input_text_multiline("##source", &text, ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), ImGuiInputTextFlags_AllowTabInput);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##2", ImVec2(child_window_width, 0.f), true);
    ImGuiExtras::markdown(text);
    ImGui::EndChild();
}

} // namespace Cool