#include "file_logger_path.hpp"
#include "Cool/Path/Path.h"

namespace Cool {

auto file_logger_path(const char* app_name) -> std::filesystem::path
{
    return Cool::Path::user_data_shared() / fmt::format("LOGS {}.txt", app_name); // Make file name start with LOG so that all log files of all apps will be next to each other in the folder
}

} // namespace Cool