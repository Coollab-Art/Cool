#include "boxer_show.hpp"
#include "Cool/Log/file_logger.hpp"
#include "spdlog/common.h"

namespace Cool {

static auto to_spdlog_level(boxer::Style style) -> spdlog::level::level_enum
{
    switch (style)
    {
    case boxer::Style::Info:
    case boxer::Style::Question:
        return spdlog::level::info;
    case boxer::Style::Warning:
        return spdlog::level::warn;
    case boxer::Style::Error:
        return spdlog::level::err;
    }
    assert(false);
    return spdlog::level::warn;
}

auto boxer_show(const char* message, const char* title, boxer::Style style, boxer::Buttons buttons) -> boxer::Selection
{
    with_file_logger([&](spdlog::logger& logger) {
        logger.log(to_spdlog_level(style), fmt::format("[{}] {}", title, message));
    });
    return boxer::show(message, title, style, buttons);
}

} // namespace Cool