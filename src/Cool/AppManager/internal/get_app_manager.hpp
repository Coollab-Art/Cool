#pragma once
#include "Cool/AppManager/AppManager.h"

namespace Cool::internal {

inline auto get_app_manager() -> AppManager*&
{
    static AppManager* instance{};
    return instance;
}

} // namespace Cool::internal