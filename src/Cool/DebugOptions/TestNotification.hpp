#pragma once
#include "ImGuiNotify/ImGuiNotify.hpp"

namespace Cool {

class TestNotification {
public:
    void imgui();

private:
    std::string       _message{"Hello"};
    ImGuiNotify::Type _type{ImGuiNotify::Type::Success};
    int               i{0};
};

} // namespace Cool
