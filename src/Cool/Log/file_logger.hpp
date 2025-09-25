#pragma once
#include "spdlog/logger.h"

namespace Cool {

/// Might do nothing if the logger is not initialised yet (e.g. if we get a File error while creating the logger, we will know not to try to log the error in the logger (because the logger failed to create so it would crash))
void with_file_logger(std::function<void(spdlog::logger&)> const& callback);

void init_log_file();

} // namespace Cool