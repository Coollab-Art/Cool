#pragma once

namespace Cool {

#if defined(__APPLE__)
#define ctrl_or_cmd "Cmd"
#define CTRL_OR_CMD "CMD"
#else
#define ctrl_or_cmd "Ctrl"
#define CTRL_OR_CMD "CTRL"
#endif

} // namespace Cool