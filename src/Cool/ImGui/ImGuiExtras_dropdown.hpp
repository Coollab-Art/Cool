#pragma once

namespace Cool::ImGuiExtras {

template<typename DropdownEntry>
auto dropdown(const char* label, const char* preview, std::vector<DropdownEntry>& entries) -> bool
{
    bool b{false};

    auto const check_for_scroll_on_item = [&]() {
        if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0.f)
        {
            bool const find_bigger = ImGui::GetIO().MouseWheel > 0.f;
            int const  sz          = static_cast<int>(entries.size());
            bool       select_next{false};
            for (int i = find_bigger ? sz - 1 : 0; find_bigger ? (i >= 0) : (i < sz); i += find_bigger ? -1 : 1)
            {
                if (select_next)
                {
                    entries[static_cast<size_t>(i)].apply_value();
                    b = true;
                    break;
                }
                select_next = entries[static_cast<size_t>(i)].is_selected();
            }
            if (!select_next) // The value is not present in the list
                if (!entries.empty())
                    entries[0].apply_value();
        }
    };

    if (ImGui::BeginCombo(label, preview))
    {
        for (auto& entry : entries)
        {
            bool const selected = entry.is_selected();
            if (ImGui::Selectable(std::string_view{entry.get_label()}.data(), selected)) // We build a string_view so that entry.get_label() can either return a string or a const char* // NOLINT(*suspicious-stringview-data-usage)
            {
                entry.apply_value();
                b = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
            // check_for_scroll_on_item(); // Disabled because when there are too many options we need to scroll to see them all, and we don't want that scroll to also change the currently selected item
        }
        ImGui::EndCombo();
    }
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY); // Make sure scrolling on the dropdown will not cause the window to scroll
    check_for_scroll_on_item();

    return b;
}

auto dropdown(const char* label, std::string* value, std::vector<std::string> const& entries) -> bool;

} // namespace Cool::ImGuiExtras