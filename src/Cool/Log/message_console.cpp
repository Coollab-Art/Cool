#include "message_console.hpp"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/icon_fmt.h"

namespace Cool {

auto message_console() -> MessageConsole&
{
    static auto instance = MessageConsole{icon_fmt("Console", ICOMOON_INFO)};
    return instance;
}

} // namespace Cool