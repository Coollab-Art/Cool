#include "Exception.h"
#include "Cool/Log/ErrorMessage.hpp"

namespace Cool {

Exception::Exception(ErrorMessage message)
    : _error_message{std::move(message)}
{}

auto Exception::what() const noexcept -> const char*
{
    return _error_message.message.c_str();
}

} // namespace Cool