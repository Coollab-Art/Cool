#pragma once
#include <map>
#include "Cool/Log/MessageId.h"
#include "INodesDefinitionUpdater.h"
#include "folder_watcher/folder_watcher.hpp"

namespace Cool {

class NodesFolderWatcher {
public:
    explicit NodesFolderWatcher(std::filesystem::path folder_path, std::string extension);

    /// Returns true iff a node definition has been added, removed or updated in the library.
    auto update(
        INodesDefinitionUpdater&,
        std::function<NodesCategoryConfig(std::filesystem::path const&)> const& make_category_config
    ) -> bool;

    auto errors_map() -> auto& { return _node_parsing_errors; }

private:
    folder_watcher::FolderWatcher              _folder_watcher{};
    std::string                                _extension;
    std::map<std::filesystem::path, MessageId> _node_parsing_errors{};
};

} // namespace Cool