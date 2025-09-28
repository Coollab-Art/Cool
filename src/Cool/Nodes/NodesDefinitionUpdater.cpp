#include "NodesDefinitionUpdater.h"
#include "Cool/Log/message_console.hpp"
#include "Cool/Nodes/NodesCategoryConfig.h"
#include "Cool/String/String.h"

namespace Cool {

static auto get_category_name(std::filesystem::path const& path, std::filesystem::path const& root) -> std::string
{
    std::string category_name = File::without_file_name(Cool::File::relative(path, root)).string();
    auto const  pos           = category_name.find_first_not_of("0123456789");
    if (pos != std::string::npos)
    {
        auto const pos2 = category_name.find_first_not_of(' ', pos);
        category_name   = Cool::String::substring(category_name, pos2, category_name.size());
    }
    return category_name.empty() ? "Unnamed Category" : category_name;
}

static auto get_category_order(std::filesystem::path const& path, std::filesystem::path const& root) -> int
{
    std::string category_name = File::without_file_name(Cool::File::relative(path, root)).string();
    auto const  pos           = category_name.find_first_not_of("0123456789");
    if (pos != std::string::npos)
        category_name = Cool::String::substring(category_name, 0, pos);
    try
    {
        return stoi(category_name);
    }
    catch (...)
    {
        return 9999;
    }
}

void NodesDefinitionUpdater::add_definition(
    std::filesystem::path const& path, std::filesystem::path const& root,
    std::function<NodesCategoryConfig(std::filesystem::path const&)> const& make_category_config
)
{
    auto const content = File::to_string(path);
    if (!content)
    {
        handle_error(path, content.error());
        return;
    }

    auto const definition = _parse_definition(path, *content);
    if (!definition)
    {
        handle_error(path, definition.error());
        return;
    }

    auto const category_name = get_category_name(path, root);

    {
        auto const category_folder = File::without_file_name(path).string();
        _library.add_definition(*definition, category_name, [&]() { return NodesCategory{category_name, make_category_config(category_folder), get_category_order(path, root)}; });
    }

    {
        // Update all nodes that use that definition
        auto lock = std::unique_lock{_graph.nodes().mutex()};
        for (auto& [_, node] : _graph.nodes())
        {
            if (node.category_name() == category_name && node.definition_name() == definition->name())
                _config.update_node_with_new_definition(node, *definition);
        }
    }
}

void NodesDefinitionUpdater::remove_definition(std::filesystem::path const& path, std::filesystem::path const& root)
{
    _library.remove_definition({
        .definition_name = File::file_name_without_extension(path).string(),
        .category_name   = get_category_name(path, root),
    });
}

auto NodesDefinitionUpdater::library_is_empty() const -> bool
{
    return _library.is_empty();
}

void NodesDefinitionUpdater::handle_error(std::filesystem::path const& definition_path, std::string const& message)
{
    message_console().send(
        _errors[definition_path], // This will create an error id if not already present in the map. This is what we want.
        {
            .type    = MessageType::Error,
            .title   = "Nodes",
            .content = fmt::format("Failed to read node \"{}\":\n{}", Cool::File::weakly_canonical(definition_path), message),
        }
    );
}

}; // namespace Cool
