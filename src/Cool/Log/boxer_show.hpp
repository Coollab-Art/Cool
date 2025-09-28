#pragma once
#include "boxer/boxer.h"

namespace Cool {

auto boxer_show(const char* message, const char* title, boxer::Style style, boxer::Buttons buttons) -> boxer::Selection;

inline auto boxer_show(const char* message, const char* title, boxer::Style style) -> boxer::Selection
{
    return boxer_show(message, title, style, boxer::kDefaultButtons);
}

inline auto show(const char* message, const char* title, boxer::Buttons buttons) -> boxer::Selection
{
    return boxer_show(message, title, boxer::kDefaultStyle, buttons);
}

inline auto show(const char* message, const char* title) -> boxer::Selection
{
    return boxer_show(message, title, boxer::kDefaultStyle, boxer::kDefaultButtons);
}

} // namespace Cool