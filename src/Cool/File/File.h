#pragma once

#include <filesystem>
#include <string_view>
#include "Cool/File/PathChecks.hpp"

namespace Cool::File {

auto make_absolute(std::filesystem::path const& path) -> std::filesystem::path;

/// Returns true iff file_path corresponds to an existing file or folder
auto exists(std::filesystem::path const& file_path) -> bool;

void remove_file(std::filesystem::path const& file_path);
void remove_folder(std::filesystem::path const& folder_path);
auto rename(std::filesystem::path const& old_path, std::filesystem::path const& new_path) -> bool;

/// Returns the name of the file + its extension (removes parent folders)
auto file_name(std::filesystem::path const& file_path) -> std::filesystem::path;

/// Returns the name of the file (removes parent folders and extension)
auto file_name_without_extension(std::filesystem::path const& file_path) -> std::filesystem::path;

/// Returns the extension of the file, preceded by a dot: something like ".png"
auto extension(std::filesystem::path const& file_path) -> std::filesystem::path;

/// Returns the file_path without its extension
auto without_extension(std::filesystem::path file_path) -> std::filesystem::path;
/// Returns the file_path with a new extension
auto with_extension(std::filesystem::path file_path, std::filesystem::path const& extension) -> std::filesystem::path;

/// Returns the folders in the path, removes the file name if there is one at the end
auto without_file_name(std::filesystem::path const& file_path) -> std::filesystem::path;

auto find_closest_existing_folder(std::filesystem::path const& file_path) -> std::filesystem::path;

auto weakly_canonical(std::filesystem::path const& path) -> std::filesystem::path;
auto relative(std::filesystem::path const& path, std::filesystem::path const& base) -> std::filesystem::path;
auto is_regular_file(std::filesystem::path const& path) -> bool;
auto is_empty(std::filesystem::path const& path) -> bool;
auto is_absolute(std::filesystem::path const& path) -> bool;
auto is_relative(std::filesystem::path const& path) -> bool;
auto last_write_time(std::filesystem::path const& path) -> std::filesystem::file_time_type;
auto equivalent(std::filesystem::path const& path1, std::filesystem::path const& path2) -> bool;

/// Returns either an expected string that contains the content of the file, or an error string containing an error message explaining why the file couldn't be read.
auto to_string(std::filesystem::path const& file_path, std::ios_base::openmode mode = std::ios_base::in) -> tl::expected<std::string, std::string>;

/// Recursively creates all the folders so that at the end `folder_path` is a valid folder path.
/// /!\ There shouldn't be any file name at the end of folder_path; it there is, then use `create_folders_for_file_if_they_dont_exist()` instead.
/// Returns true iff the folders now exist (be it because they already existed or because they have been successfully created).
[[nodiscard("You can only use the folders if this function returns true")]] auto create_folders_if_they_dont_exist(std::filesystem::path const& folder_path) -> bool;

/// Recursively creates all the folders so that at the end `file_path` is a valid file path.
/// Returns true iff the folders now exist (be it because they already existed or because they have been successfully created).
[[nodiscard("You can only use the file if this function returns true")]] auto create_folders_for_file_if_they_dont_exist(std::filesystem::path const& file_path) -> bool;

/// Recursively creates all the folders and the file so that at the end `file_path` is a valid file path.
/// Returns true iff the file now exists.
auto create_file_if_it_doesnt_exist(std::filesystem::path const& file_path) -> bool;

/// Returns true iff the copy succeeded.
auto copy_file(std::filesystem::path const& from, std::filesystem::path const& to) -> bool;

/// Returns a file name that isn't in use yet; it will be file_name, eventually postfixed with a number in parenthesis
/// folder_path: The folder where you want the file to be created
/// file_name: The name you would like to give to the file
/// extension: The extension of the file, e.g. ".png"
auto find_available_name(std::filesystem::path const& folder_path, std::filesystem::path const& file_name, std::filesystem::path const& extension, PathChecks const& path_checks) -> std::filesystem::path;
auto find_available_path(std::filesystem::path const& path, PathChecks const& path_checks) -> std::filesystem::path;
/// Usefull when you know that a file will be created soon (e.g. by a Task in 1 or 2 seconds) and want to prevent others from trying to create a file with the same path
void mark_file_path_unavailable(std::filesystem::path const& path);

/// Overwrites the content of the file and set it to `content`.
/// Creates the file if it doesn't exist yet.
void set_content(std::filesystem::path const& file_path, std::string_view content);
/// Does nothing if the file doesn't exist
void set_time_of_last_change_to_now(std::filesystem::path const& file_path);

struct folder_dialog_args {
    /// Folder that the dialog window should open at. Leave blank for default (platform-specific) behaviour.
    std::filesystem::path initial_folder = "";
};
/// Opens a folder dialog UI, and returns the selected path, or nullopt if the dialog is canceled / closed by the user.
auto folder_dialog(folder_dialog_args const& = {}) -> std::optional<std::filesystem::path>;

struct file_dialog_args {
    /// A set of filters for the file types that should be selectable. Something like { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } }.
    /// You can find predefined filters in <Cool/NfdFileFilter/NfdFileFilter.h>.
    std::vector<nfdfilteritem_t> file_filters = {};
    /// Folder that the dialog window should open at. Leave blank for default (platform-specific) behaviour.
    std::filesystem::path initial_folder = "";
};
/// Opens a file dialog UI, and returns the selected path, or nullopt if the dialog is canceled / closed by the user.
auto file_opening_dialog(file_dialog_args const& = {}) -> std::optional<std::filesystem::path>;
/// Opens a file dialog UI, and returns the selected path, or nullopt if the dialog is canceled / closed by the user.
auto file_saving_dialog(file_dialog_args const& = {}) -> std::optional<std::filesystem::path>;

} // namespace Cool::File