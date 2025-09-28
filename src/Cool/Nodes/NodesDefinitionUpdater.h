#pragma once
#include <utility>
#include "Cool/Log/MessageId.h"
#include "INodesDefinitionUpdater.h"
#include "NodesConfig.h"
#include "NodesGraph.h"
#include "NodesLibrary.h"

namespace Cool {

using NodeDefinitionParser = std::function<tl::expected<NodeDefinition, std::string>(std::filesystem::path const&, std::string const&)>;

class NodesDefinitionUpdater : public INodesDefinitionUpdater {
public:
    NodesDefinitionUpdater(
        NodesConfig&                                config,
        NodesGraph&                                 graph,
        NodesLibrary&                               library,
        NodeDefinitionParser                        parse_definition,
        std::map<std::filesystem::path, MessageId>& errors
    )
        : _config{config}
        , _graph{graph}
        , _library{library}
        , _parse_definition{std::move(parse_definition)}
        , _errors{errors}
    {}

    void add_definition(
        std::filesystem::path const& path, std::filesystem::path const& root,
        std::function<NodesCategoryConfig(std::filesystem::path const&)> const& make_category_config
    ) override;
    void               remove_definition(std::filesystem::path const& path, std::filesystem::path const& root) override;
    [[nodiscard]] auto library_is_empty() const -> bool override;

private:
    void handle_error(std::filesystem::path const& definition_path, std::string const& message);

private:
    NodesConfig&                                _config; // Take a non-const ref to force the compiler to detect when we are passing a concrete config and creating a type-erased Cool::NodesConfig on the fly, that is a temporary variable and we cannot keep a reference to it (For a const& the compiler would not complain though we are taking a reference to a temporary). Users must create a Cool::NodesConfig on their side and then pass it to us.
    NodesGraph&                                 _graph;
    NodesLibrary&                               _library;
    NodeDefinitionParser                        _parse_definition;
    std::map<std::filesystem::path, MessageId>& _errors;
};

}; // namespace Cool
