#include "NodesLibrary.h"
#include <Cool/String/String.h>
#include <imgui.h>
#include <charconv>
#include <string>
#include <wafl/wafl.hpp>
#include "Cool/ImGui/ImGuiExtras.h"

namespace Cool {

namespace internal {
auto name_matches_filter(std::string const& name, std::string const& filter) -> bool
{
    if (filter.empty())
        return true;

    auto const name_lower   = Cool::String::to_lower(name);
    auto const filter_lower = Cool::String::to_lower(filter);
    return name_lower.find(filter_lower) != std::string::npos;
}
} // namespace internal

auto NodesLibrary::get_category(std::string const& category_name) const -> NodesCategory const*
{
    auto const it = std::find_if(_categories.begin(), _categories.end(), [&](NodesCategory const& cat) {
        return cat.name() == category_name;
    });

    if (it != _categories.end())
        return &*it;
    return nullptr;
}

auto NodesLibrary::get_category(std::string const& category_name) -> NodesCategory*
{
    auto const it = std::find_if(_categories.begin(), _categories.end(), [&](NodesCategory const& cat) {
        return cat.name() == category_name;
    });

    if (it != _categories.end())
        return &*it;
    return nullptr;
}

auto NodesLibrary::imgui_nodes_menu(std::string const& nodes_filter, MaybeDisableNodeDefinition const& maybe_disable, bool select_first, bool open_all_categories, bool menu_just_opened) const -> std::optional<NodeDefinitionAndCategoryName>
{
    // TODO :
    // for(group  : groups){}
    // beginColumn
    // for (auto& category : group.categories)

    size_t i = 0;

    std::string strb = "b" + std::to_string(i);
    const char* b    = strb.c_str();
    std::string strc = "c" + std::to_string(i);
    const char* c    = strc.c_str();
    if (ImGui::BeginTable("columns_table", 4))
    {
        auto const end_columns_table_automatically = sg::make_scope_guard([]() { ImGui::EndTable(); });

        ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, 370.0f);
        ImGui::TableSetupColumn("column2", ImGuiTableColumnFlags_WidthFixed, 370.0f);
        ImGui::TableSetupColumn("column3", ImGuiTableColumnFlags_WidthFixed, 370.0f);
        ImGui::TableSetupColumn("column4", ImGuiTableColumnFlags_WidthFixed, 370.0f);
        ImGui::TableNextRow();
        for (int column = 0; column < 4; column++)
        {
            ImGui::TableSetColumnIndex(column);
            if (ImGui::BeginTable("families_table", 1))
            {
                auto const end_families_table_automatically = sg::make_scope_guard([]() { ImGui::EndTable(); });

                ImGui::TableSetupColumn(b, ImGuiTableColumnFlags_WidthFixed, 360.0f);
                for (int family_row = 0; family_row < 4; family_row++)
                {
                    ImGui::TableNextRow();

                    for (int family_col = 0; family_col < 1; family_col++)
                    {
                        ImGui::TableSetColumnIndex(family_col);
                        if (ImGui::BeginTable("categories_table", 1))
                        {
                            auto const end_categories_table_automatically = sg::make_scope_guard([]() { ImGui::EndTable(); });

                            ImGui::TableSetupColumn(c, ImGuiTableColumnFlags_WidthFixed, 380.0f);
                            for (int category_row = 0; category_row < 4; category_row++)
                            {
                                ImGui::TableNextRow();

                                //////// Category display /////////
                                for (int category_col = 0; category_col < 1; category_col++)
                                {
                                    ImGui::TableSetColumnIndex(category_col);

                                    if (i < _categories.size())
                                    {
                                        auto category = _categories[i];

                                        ImGui::PushID(&category);
                                        auto const pop_automatically = sg::make_scope_guard([]() { ImGui::PopID(); });

                                        bool is_open    = false;
                                        bool is_visible = true;
                                        if (!nodes_filter.empty())
                                        {
                                            is_visible = false;
                                            for (NodeDefinition const& def : category.definitions())
                                            {
                                                if (internal::name_matches_filter(def.name(), nodes_filter))
                                                {
                                                    is_open    = true;
                                                    is_visible = true;
                                                }
                                            }
                                        }

                                        if (!is_visible)
                                        {
                                            i++;
                                            continue;
                                        }

                                        if (open_all_categories || menu_just_opened)
                                            ImGui::SetNextItemOpen(is_open);

                                        ImGui::PushID(13452);
                                        bool const collapsing_header_open = ImGuiExtras::colored_collapsing_header(category.name(), category.config().color());
                                        ImGui::PopID();

                                        category.config().imgui_popup();

                                        if (collapsing_header_open)
                                        {
                                            for (NodeDefinition const& def : category.definitions())
                                            {
                                                if (!internal::name_matches_filter(def.name(), nodes_filter))
                                                    continue;

                                                auto selected_definition = std::optional<NodeDefinitionAndCategoryName>{};
                                                Cool::ImGuiExtras::disabled_if(maybe_disable(def, category), [&]() {
                                                    if (select_first || ImGui::Selectable(def.name().c_str(), false, ImGuiSelectableFlags_SpanAllColumns /* HACK to work around a bug in ImGui (https://github.com/ocornut/imgui/issues/8203)*/))
                                                        selected_definition = NodeDefinitionAndCategoryName{def, category.name()};
                                                });

                                                if (selected_definition.has_value())
                                                    return selected_definition;
                                            }
                                        }

                                        ////// End Category display //////

                                        i++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return std::nullopt;
}

void NodesLibrary::add_definition(
    NodeDefinition const&                 definition,
    std::string const&                    category_name,
    std::function<NodesCategory()> const& make_category // We take a function because we want to delay the creation of the category until we are sure we need to create one, which is rare (and otherwise would cause warning when two folders correspond to the same category, but only one of them contains the _category_config.json, which happens when reading nodes from both the app's root folder and the user-data folder)
)
{
    // Add definition to the corresponding category if it exists
    // TODO parcourir les familles
    for (auto& category : _categories)
    {
        if (category.name() != category_name)
            continue;

        category.definitions().push_back(definition);
        category.sort();
        return;
    }

    // Add new category if not found
    _categories.push_back(make_category());
    _categories.back().definitions().push_back(definition);
    std::sort(
        _categories.begin(), _categories.end(),
        [](NodesCategory const& c1, NodesCategory const& c2) {
            return c1.order() < c2.order();
        }
    );
}

void NodesLibrary::remove_definition(NodeDefinitionIdentifier const& identifier)
{
    auto* category = get_category(identifier.category_name);
    if (!category)
        return;

    // Remove the node from the definitions
    std::erase_if(category->definitions(), [&](NodeDefinition const& def) { return def.name() == identifier.definition_name; });

    // Remove the category from _categories if is now empty.
    if (category->definitions().empty())
    {
        std::erase_if(_categories, [&](NodesCategory const& category) {
            return category.name() == identifier.category_name;
        });
    }
}

auto NodesLibrary::is_empty() const -> bool
{
    return _categories.empty();
}

} // namespace Cool