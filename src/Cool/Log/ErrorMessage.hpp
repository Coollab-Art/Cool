#pragma once
#include "ClipboardContent.hpp"

namespace Cool {

struct [[nodiscard]] ErrorMessage {
    std::string                   message{};
    std::vector<ClipboardContent> clipboard_contents{};
};

} // namespace Cool