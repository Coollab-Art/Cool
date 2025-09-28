#pragma once

namespace Cool {

auto file_logger_path(const char* app_name = COOL_APP_NAME) -> std::filesystem::path;

}