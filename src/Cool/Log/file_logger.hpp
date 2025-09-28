#pragma once
#include "spdlog/logger.h"

namespace Cool {

auto file_logger() -> spdlog::logger&;

}