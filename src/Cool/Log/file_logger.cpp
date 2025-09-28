#include "file_logger.hpp"
#include "Cool/Dump/gen_dump_string.h"
#include "Cool/File/File.h"
#include "file_logger_path.hpp"
#include "spdlog/sinks/basic_file_sink.h"

namespace Cool {

static auto make_logger()
{
    auto const path = file_logger_path();

    Cool::File::set_content(path, Cool::gen_dump_string() + "\n\n");

    auto logger = spdlog::basic_logger_mt("file_logger", path.string());
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    return logger;
}

auto file_logger() -> spdlog::logger&
{
    static auto logger = make_logger();
    return *logger;
}

} // namespace Cool