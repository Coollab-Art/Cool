#if defined(COOL_OPENGL)
#include "preprocess_shader_source.h"
#include <Cool/File/File.h>
#include <Cool/Path/Path.h>
#include <Cool/RegExp/RegExp.h>
#include <Cool/String/String.h>
#include <exception>
#include <filesystem>
#include <sstream>
#include "Cool/Utils/getline.hpp"

namespace Cool::OpenGL {

static auto preprocess_shader_source_impl(std::string_view source, std::vector<std::filesystem::path>& included_paths) -> tl::expected<std::string, std::string>;

/// If line is a `#include` line, returns the content of the included file
/// (or an error if the file couldn't be read).
/// Otherwise just returns the line that was passed in.
static auto line_or_include(std::string const& line, std::vector<std::filesystem::path>& included_paths) -> tl::expected<std::string, std::string>
{
    auto const path = RegExp::file_path_to_include(line);
    if (!path.has_value())
        return line;

    // Early return if path has already been included
    try
    {
        auto const canonical_path = Cool::File::weakly_canonical(*path);

        if (std::find(included_paths.begin(), included_paths.end(), canonical_path) != included_paths.end())
            return fmt::format("// Path {} has already been included.", *path);

        included_paths.push_back(canonical_path);
    }
    catch (std::exception& e)
    {
        return tl::make_unexpected(fmt::format(
            "Failed to #include file \"{}\":\n{}",
            *path, e.what()
        ));
    }

    auto const file_content = File::to_string(*path);
    if (!file_content)
    {
        return tl::make_unexpected(fmt::format(
            "Failed to #include file \"{}\":\n{}",
            *path, file_content.error()
        ));
    }

    auto const preprocessed_content = preprocess_shader_source_impl(*file_content, included_paths);
    if (!preprocessed_content)
        return preprocessed_content;

    return fmt::format(
        "// BEGIN INCLUDE {0}\n{1}\n// END INCLUDE {0}",
        *path,
        *preprocessed_content
    );
}

static auto preprocess_shader_source_impl(std::string_view source, std::vector<std::filesystem::path>& included_paths) -> tl::expected<std::string, std::string>
{
    std::istringstream stream{std::string(source)};
    std::ostringstream output;

    std::string line;
    while (Cool::getline(stream, line))
    {
        String::replace_all_inplace(line, "_COOL_RES_", Path::cool_res().string());
        String::replace_all_inplace(line, "_ROOT_FOLDER_", Path::root().string());
        String::replace_all_inplace(line, "_USER_DATA_", Path::user_data().string());
        auto const content = line_or_include(line, included_paths);
        if (!content)
            return content;
        output << *content << '\n';
    }

    return output.str();
}

auto preprocess_shader_source(std::string_view source) -> tl::expected<std::string, std::string>
{
    auto included_paths = std::vector<std::filesystem::path>{};
    return preprocess_shader_source_impl(source, included_paths);
}

} // namespace Cool::OpenGL

#endif