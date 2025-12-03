#include "NodesFolderWatcher.h"
#include <utility>
#include "Cool/File/File.h"
#include "Cool/Log/message_console.hpp"

namespace Cool {

NodesFolderWatcher::NodesFolderWatcher(std::filesystem::path folder_path, std::string extension)
    : _folder_watcher{std::move(folder_path)}
    , _extension{std::move(extension)}
{}

auto NodesFolderWatcher::update(
    INodesDefinitionUpdater&                                                updater,
    std::function<NodesCategoryConfig(std::filesystem::path const&)> const& make_category_config
) -> bool
{
    // TODO do update with a task to avoid lag spikes (NB : this would be a good candidate to check if turning tasks into coroutines makes them slower)
    // TODO make a TaskInspector where we can see the name of all the running tasks (and how long they have been running for etc.)
    // TODO checking folders could be an infinite task, a corountinne that does a while true, and then a for on each file in the directory
    // NB: if we split this into many small tasks we can run them on all the threads if they have nothing else to do (but its hard to split a for in bit size tasks, and also we don't really need that extra performance gain)
    bool       has_changed                      = false;
    auto const clear_errors_and_check_extension = [&](std::filesystem::path const& path) {
        if (path.extension() != _extension)
            return false;

        message_console().remove(_node_parsing_errors[path]);
        has_changed = true;
        return true;
    };

    _folder_watcher.update({
        .on_file_added = [&](std::filesystem::path const& path) {
            if (!clear_errors_and_check_extension(path))
                return;

            updater.add_definition(path, _folder_watcher.get_folder_path(), make_category_config); },

        .on_file_removed = [&](std::filesystem::path const& path) {
            if (!clear_errors_and_check_extension(path))
                return;

            updater.remove_definition(path, _folder_watcher.get_folder_path()); },

        .on_file_changed = [&](std::filesystem::path const& path) {
            if (!clear_errors_and_check_extension(path))
                return;

            updater.remove_definition(path, _folder_watcher.get_folder_path());
            updater.add_definition(path, _folder_watcher.get_folder_path(), make_category_config); },

        .on_invalid_folder_path = [&](std::filesystem::path const& path) {
            has_changed = true;
            Log::error("Nodes", fmt::format("Nodes folder does not exist. \"{}\"", Cool::File::weakly_canonical(path))); },
    });

    return has_changed;
}

}; // namespace Cool