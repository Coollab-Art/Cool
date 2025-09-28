#include "reset_log_file_if_not_already_reset.hpp"
#include "file_logger.hpp"

namespace Cool {

void reset_log_file_if_not_already_reset()
{
    file_logger(); // This will make sure the logger gets created, which resets the content of the log file
}

} // namespace Cool