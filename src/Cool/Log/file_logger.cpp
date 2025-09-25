#include "file_logger.hpp"
#include <exception>
#include "Cool/Dump/gen_dump_string.hpp"
#include "Cool/File/File.h"
#include "Cool/Log/Log.hpp"
#include "file_logger_path.hpp"
#include "spdlog/sinks/basic_file_sink.h"

namespace Cool {

static auto make_logger() -> std::shared_ptr<spdlog::logger>
{
    auto const path = file_logger_path();

    File::set_content(path, gen_dump_string() + "\n\n");

    try
    {
        auto logger = spdlog::basic_logger_mt("file_logger", path.string());
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        return logger;
    }
    catch (std::exception const& e)
    {
        Log::internal_error("Failed to create log file", e.what());
        return nullptr;
    }
}

static auto file_logger() -> spdlog::logger*
{
    static std::shared_ptr<spdlog::logger> logger = make_logger();
    return logger.get();
}

static auto file_logger_is_init() -> bool&
{
    static auto instance = false;
    return instance;
}

void init_log_file()
{
    file_logger(); // This will make sure the logger gets created, which resets the content of the log file
    file_logger_is_init() = true;
}

void with_file_logger(std::function<void(spdlog::logger&)> const& callback)
{
    if (!file_logger_is_init() || !file_logger())
        return; // Do nothing if the logger is not initialised yet (e.g. if we get a File error while creating the logger, we must not try to log the error in the logger (because the logger failed to create so it would crash))

    callback(*file_logger());
    file_logger()->flush(); // We flush as soon as we log a message, to make sure that if the app crashes we won't lose any logs that haven't been flushed yet
}

} // namespace Cool