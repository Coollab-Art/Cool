#include "ImGuiExtras_dropdown.hpp"

namespace Cool::ImGuiExtras {

auto dropdown(const char* label, std::string* value, std::vector<std::string> const& entries) -> bool
{
    struct DropdownEntry {
        std::string*       value;
        std::string const* entry;

        auto get_label() const -> const char* { return entry->c_str(); }
        auto is_selected() const -> bool { return *value == *entry; }
        void apply_value() const { *value = *entry; }
    };

    auto actual_entries = std::vector<DropdownEntry>{};
    actual_entries.reserve(entries.size());
    for (auto const& entry : entries)
        actual_entries.push_back({value, &entry});

    return dropdown(label, value->c_str(), actual_entries);
}

} // namespace Cool::ImGuiExtras