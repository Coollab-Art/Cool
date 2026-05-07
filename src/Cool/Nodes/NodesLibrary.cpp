#include "NodesLibrary.h"
#include <Cool/String/String.h>
#include <imgui.h>
#include <algorithm>
#include <string>
#include <vector>
#include "Cool/ImGui/ImGuiExtras.h"

namespace Cool {

namespace internal {
struct VisibleCategory {
    size_t index;
    bool   has_matching_definition;
};

struct PreparedNodesMenu {
    std::vector<VisibleCategory> visible_categories;
    float                        required_column_width;
};

constexpr float k_MinColumnWidth          = 260.f;
constexpr float k_NodeLabelExtraWidth     = 28.f;
constexpr float k_CategoryLabelExtraWidth = 34.f;
constexpr float k_InterColumnGap          = 10.f;
constexpr float k_WidthSafetyMargin       = 4.f;

auto name_matches_filter(std::string const& name, std::string const& filter) -> bool
{
    if (filter.empty())
        return true;

    auto const name_lower   = Cool::String::to_lower(name);
    auto const filter_lower = Cool::String::to_lower(filter);
    return name_lower.find(filter_lower) != std::string::npos;
}

static auto is_definition_visible(NodeDefinition const& def, std::string const& nodes_filter) -> bool
{
    return nodes_filter.empty() || name_matches_filter(def.name(), nodes_filter);
}

static auto definition_label_width(NodeDefinition const& def, ImGuiStyle const& style) -> float
{
    return ImGui::CalcTextSize(def.name().c_str()).x
           + 2.f * style.FramePadding.x
           + style.ItemSpacing.x
           + k_NodeLabelExtraWidth;
}

static auto category_label_width(NodesCategory const& category, ImGuiStyle const& style) -> float
{
    return ImGui::CalcTextSize(category.name().c_str()).x
           + 2.f * style.FramePadding.x
           + style.ItemSpacing.x
           + k_CategoryLabelExtraWidth; // Arrow + breathing room for collapsible header
}

static auto prepare_nodes_menu(std::vector<NodesCategory> const& categories, std::string const& nodes_filter, ImGuiStyle const& style) -> PreparedNodesMenu
{
    auto prepared = PreparedNodesMenu{
        .visible_categories    = {},
        .required_column_width = k_MinColumnWidth,
    };
    prepared.visible_categories.reserve(categories.size());

    for (size_t idx = 0; idx < categories.size(); ++idx)
    {
        auto const& category = categories[idx];

        bool has_matching_definition = false;
        for (NodeDefinition const& def : category.definitions())
        {
            prepared.required_column_width = std::max(prepared.required_column_width, definition_label_width(def, style));
            if (!nodes_filter.empty() && name_matches_filter(def.name(), nodes_filter))
                has_matching_definition = true;
        }

        prepared.required_column_width = std::max(prepared.required_column_width, category_label_width(category, style));

        if (nodes_filter.empty() || has_matching_definition)
            prepared.visible_categories.push_back({idx, has_matching_definition});
    }

    return prepared;
}

static auto compute_number_of_columns(PreparedNodesMenu const& prepared_menu, ImGuiStyle const& style) -> int
{
    auto const column_total_width = prepared_menu.required_column_width + 2.f * style.CellPadding.x;
    auto const available_width    = std::max(0.f, ImGui::GetContentRegionAvail().x - style.ScrollbarSize - k_WidthSafetyMargin);
    auto const max_columns        = std::max(1, static_cast<int>((available_width + k_InterColumnGap) / (column_total_width + k_InterColumnGap)));
    return std::min(max_columns, static_cast<int>(prepared_menu.visible_categories.size()));
}

static auto render_node_definitions(
    NodesCategory&                    category,
    std::string const&                nodes_filter,
    MaybeDisableNodeDefinition const& maybe_disable,
    bool                              select_first
) -> std::optional<NodeDefinitionAndCategoryName>
{
    for (NodeDefinition const& def : category.definitions())
    {
        if (!is_definition_visible(def, nodes_filter))
            continue;

        auto selected_definition = std::optional<NodeDefinitionAndCategoryName>{};
        Cool::ImGuiExtras::disabled_if(maybe_disable(def, category), [&]() {
            if (select_first || ImGui::Selectable(def.name().c_str()))
                selected_definition = NodeDefinitionAndCategoryName{def, category.name()};
        });

        if (selected_definition.has_value())
            return selected_definition;
    }

    return std::nullopt;
}

static auto render_category(
    NodesCategory                     category,
    VisibleCategory const&            visible_entry,
    std::string const&                nodes_filter,
    MaybeDisableNodeDefinition const& maybe_disable,
    bool                              select_first,
    bool                              should_force_open
) -> std::optional<NodeDefinitionAndCategoryName>
{
    ImGui::PushID(&category);
    auto const pop_automatically = sg::make_scope_guard([]() { ImGui::PopID(); });

    if (should_force_open)
        ImGui::SetNextItemOpen(visible_entry.has_matching_definition);

    ImGui::PushID(13452);
    bool const collapsing_header_open = ImGuiExtras::colored_collapsing_header(category.name(), category.config().color());
    ImGui::PopID();

    category.config().imgui_popup();

    if (!collapsing_header_open)
        return std::nullopt;

    return render_node_definitions(category, nodes_filter, maybe_disable, select_first);
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
    auto const& style = ImGui::GetStyle();

    auto const prepared_menu = internal::prepare_nodes_menu(_categories, nodes_filter, style);
    if (prepared_menu.visible_categories.empty())
        return std::nullopt;

    int const  number_of_columns = internal::compute_number_of_columns(prepared_menu, style);
    auto const rows_per_column   = (prepared_menu.visible_categories.size() + static_cast<size_t>(number_of_columns) - 1) / static_cast<size_t>(number_of_columns);
    bool const should_force_open = (open_all_categories || menu_just_opened) && !nodes_filter.empty();

    int const table_columns = number_of_columns * 2 - 1;
    if (ImGui::BeginTable(
            "nodes_categories_columns",
            table_columns,
            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoPadOuterX
        ))
    {
        auto const end_columns_table_automatically = sg::make_scope_guard([]() { ImGui::EndTable(); });

        for (int col = 0; col < number_of_columns; ++col)
        {
            ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 1.f);
            if (col < number_of_columns - 1)
                ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, internal::k_InterColumnGap);
        }

        ImGui::TableNextRow();
        for (int col = 0; col < number_of_columns; ++col)
        {
            ImGui::TableSetColumnIndex(col * 2);

            auto const begin = static_cast<size_t>(col) * rows_per_column;
            auto const end   = std::min(begin + rows_per_column, prepared_menu.visible_categories.size());

            for (size_t list_idx = begin; list_idx < end; ++list_idx)
            {
                auto const visible_entry             = prepared_menu.visible_categories[list_idx];
                auto const maybe_selected_definition = internal::render_category(
                    _categories[visible_entry.index],
                    visible_entry,
                    nodes_filter,
                    maybe_disable,
                    select_first,
                    should_force_open
                );
                if (maybe_selected_definition.has_value())
                    return maybe_selected_definition;
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