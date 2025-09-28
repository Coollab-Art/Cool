#include "File.h"
#include <exception>
#include <filesystem>
#include <fstream>
#include "Cool/File/File.h"
#include "Cool/String/String.h"
#include "nfd.hpp"

namespace Cool::File {

auto make_absolute(std::filesystem::path const& path) -> std::filesystem::path
{
    try
    {
        return path.empty() ? "" : std::filesystem::absolute(path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to make absolute path from \"{}\":\n{}", path, e.what()));
        return path;
    }
}

auto exists(std::filesystem::path const& file_path) -> bool
{
    try
    {
        return std::filesystem::exists(file_path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to check if \"{}\" exists:\n{}", file_path, e.what()));
        return false;
    }
}

void remove_file(std::filesystem::path const& file_path)
{
    try
    {
        std::filesystem::remove(file_path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to remove file \"{}\":\n{}", file_path, e.what()));
    }
}

void remove_folder(std::filesystem::path const& folder_path)
{
    try
    {
        std::filesystem::remove_all(folder_path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to remove folder \"{}\":\n{}", folder_path, e.what()));
    }
}

auto rename(std::filesystem::path const& old_path, std::filesystem::path const& new_path) -> bool
{
    try
    {
        std::filesystem::rename(old_path, new_path);
        return true;
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to rename \"{}\" as \"{}\":\n{}", old_path, new_path, e.what()));
        return false;
    }
}

auto file_name(std::filesystem::path const& file_path) -> std::filesystem::path
{
    return file_path.filename();
}

auto file_name_without_extension(std::filesystem::path const& file_path) -> std::filesystem::path
{
    return file_path.stem();
}

auto extension(std::filesystem::path const& file_path) -> std::filesystem::path
{
    return file_path.extension();
}

auto without_extension(std::filesystem::path file_path) -> std::filesystem::path
{
    return file_path.replace_extension();
}

auto with_extension(std::filesystem::path file_path, std::filesystem::path const& extension) -> std::filesystem::path
{
    file_path.replace_extension(extension);
    return file_path;
}

auto without_file_name(std::filesystem::path const& file_path) -> std::filesystem::path
{
    if (!file_path.has_filename())
        return file_path;
    return file_path.parent_path();
}

auto find_closest_existing_folder(std::filesystem::path const& file_path) -> std::filesystem::path
{
    auto previous_length = file_path.string().size();

    auto path = Cool::File::without_file_name(file_path);
    while (!Cool::File::exists(path) && previous_length > path.string().size())
        path = path.parent_path();
    return path;
}

auto weakly_canonical(std::filesystem::path const& path) -> std::filesystem::path
{
    try
    {
        return std::filesystem::weakly_canonical(path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to get canonical path for \"{}\":\n{}", path, e.what()));
        return path;
    }
}

auto relative(std::filesystem::path const& path, std::filesystem::path const& base) -> std::filesystem::path
{
    try
    {
        return std::filesystem::relative(path, base);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to make path \"{}\" relative to \"{}\":\n{}", path, base, e.what()));
        return path;
    }
}

auto is_regular_file(std::filesystem::path const& path) -> bool
{
    try
    {
        return std::filesystem::is_regular_file(path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to check if \"{}\" is a regular file:\n{}", path, e.what()));
        return false;
    }
}

auto is_empty(std::filesystem::path const& path) -> bool
{
    try
    {
        return std::filesystem::is_empty(path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to check if \"{}\" is empty:\n{}", path, e.what()));
        return false;
    }
}

auto is_absolute(std::filesystem::path const& path) -> bool
{
    try
    {
        return path.is_absolute();
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to check if \"{}\" is an absolute path:\n{}", path, e.what()));
        return true;
    }
}

auto is_relative(std::filesystem::path const& path) -> bool
{
    try
    {
        return path.is_relative();
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to check if \"{}\" is a relative path:\n{}", path, e.what()));
        return true;
    }
}

auto last_write_time(std::filesystem::path const& path) -> std::filesystem::file_time_type
{
    try
    {
        return std::filesystem::last_write_time(path);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to get last write time for \"{}\":\n{}", path, e.what()));
        return {};
    }
}

auto equivalent(std::filesystem::path const& path1, std::filesystem::path const& path2) -> bool
{
    try
    {
        return File::weakly_canonical(path1) == File::weakly_canonical(path2) // equivalent() throws when one of the two paths doesn't exist, so we add this check to handle the case of paths that don't exist
               || std::filesystem::equivalent(path1, path2);
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to check if paths are equivalent \"{}\" and \"{}\":\n{}", path1, path2, e.what()));
        return false;
    }
}

auto to_string(std::filesystem::path const& file_path, std::ios_base::openmode mode) -> tl::expected<std::string, std::string>
{
    // Thanks to https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
    std::ifstream stream(file_path, mode);
    if (!stream.is_open())
    {
        return tl::make_unexpected(
            fmt::format("Failed to open file \"{}\"", file_path)
        );
    }
    stream.seekg(0, std::ios::end);
    std::string str;
    str.reserve(static_cast<size_t>(stream.tellg()));
    stream.seekg(0, std::ios::beg);
    str.assign(
        (std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>()
    );
    return str;
}

auto create_folders_if_they_dont_exist(std::filesystem::path const& folder_path) -> bool
{
    if (File::exists(folder_path))
        return true;

    try
    {
        std::filesystem::create_directories(folder_path);
        return true;
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to create folder \"{}\":\n{}", folder_path, e.what()));
        return false;
    }
}

auto create_folders_for_file_if_they_dont_exist(std::filesystem::path const& file_path) -> bool
{
    return create_folders_if_they_dont_exist(without_file_name(file_path));
}

auto create_file_if_it_doesnt_exist(std::filesystem::path const& file_path) -> bool
{
    if (File::exists(file_path))
        return true;
    if (!create_folders_for_file_if_they_dont_exist(file_path))
        return false;

    auto file = std::ofstream{file_path};
    return file.is_open();
}

auto copy_file(std::filesystem::path const& from, std::filesystem::path const& to) -> bool
{
    if (!create_folders_for_file_if_they_dont_exist(to))
        return false;

    try
    {
        std::filesystem::copy_file(from, to);
        return true;
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to copy file from \"{}\" to \"{}\":\n{}", from, to, e.what()));
        return false;
    }
}

auto find_available_name(
    std::filesystem::path const& folder_path,
    std::filesystem::path const& file_name,
    std::filesystem::path const& extension,
    PathChecks const&            path_checks
) -> std::filesystem::path // NOLINT(*easily-swappable-parameters)
{
    std::string const name = Cool::File::without_extension(file_name).string();
    // Split file_name into a number in parenthesis and the base_name that is before those parenthesis
    auto [k, base_name] = [&]() {
        if (auto const pos = name.find_last_of('(');
            pos != std::string::npos)
        {
            auto const end_pos = name.find_last_of(')');
            try
            {
                return std::make_pair(
                    std::stoi(Cool::String::substring(name, pos + 1, end_pos + 1)),
                    Cool::String::substring(name, 0, pos)
                );
            }
            catch (std::exception const&)
            {
            }
        }
        return std::make_pair(0, name);
    }();

    // Find an available name
    auto out_name = file_name;
    while (!path_checks.is_valid(File::with_extension(folder_path / out_name, extension)))
    {
        out_name = base_name + "(" + std::to_string(k) + ")";
        k++;
    }
    return out_name;
}

auto find_available_path(std::filesystem::path const& path, PathChecks const& path_checks) -> std::filesystem::path
{
    auto const folder    = without_file_name(path);
    auto const file      = file_name_without_extension(path);
    auto const extension = File::extension(path);
    return with_extension(folder / find_available_name(folder, file, extension, path_checks), extension);
}

void mark_file_path_unavailable(std::filesystem::path const& path)
{
    create_file_if_it_doesnt_exist(path);
}

void set_content(std::filesystem::path const& file_path, std::string_view content)
{
    if (!create_folders_for_file_if_they_dont_exist(file_path))
        return;

    auto file = std::ofstream{file_path};
    if (!file.is_open())
        return;

    file << content;
}

void set_time_of_last_change_to_now(std::filesystem::path const& file_path)
{
    try
    {
        std::filesystem::last_write_time(file_path, std::filesystem::file_time_type::clock::now());
    }
    catch (std::exception const& e)
    {
        Log::internal_warning("File", fmt::format("Failed to set time of last change to now for \"{}\":\n{}", file_path, e.what()));
    }
}

auto folder_dialog(folder_dialog_args const& args) -> std::optional<std::filesystem::path>
{
    auto        path   = NFD::UniquePath{};
    nfdresult_t result = NFD::PickFolder(path, make_absolute(args.initial_folder).string().c_str());
    if (result != NFD_OKAY)
        return std::nullopt;
    return std::filesystem::path{path.get()};
}

auto file_opening_dialog(file_dialog_args const& args) -> std::optional<std::filesystem::path>
{
    auto        path   = NFD::UniquePath{};
    nfdresult_t result = NFD::OpenDialog(
        path,
        args.file_filters.data(),
        static_cast<nfdfiltersize_t>(args.file_filters.size()),
        make_absolute(args.initial_folder).string().c_str()
    );
    if (result != NFD_OKAY)
        return std::nullopt;
    return std::filesystem::path{path.get()};
}

auto file_saving_dialog(file_dialog_args const& args) -> std::optional<std::filesystem::path>
{
    auto        path   = NFD::UniquePath{};
    nfdresult_t result = NFD::SaveDialog(
        path,
        args.file_filters.data(),
        static_cast<nfdfiltersize_t>(args.file_filters.size()),
        make_absolute(args.initial_folder).string().c_str()
    );
    if (result != NFD_OKAY)
        return std::nullopt;
    return std::filesystem::path{path.get()};
}

} // namespace Cool::File
