#pragma once
#include "ClipboardContent.hpp"

namespace Cool {

enum class MessageType : uint8_t {
    Info,
    Warning,
    Error,
};

struct Message {
    MessageType                   type{MessageType::Error};
    std::string                   title;
    std::string                   content;
    std::vector<ClipboardContent> clipboard_contents{}; // Controls the string that will be copied to the clipboard when users click on the message. If left empty, the whole `message` will be copied to the clipboard.
};

} // namespace Cool