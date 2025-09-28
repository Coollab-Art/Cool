#pragma once
#include <exception>
#include "Cool/Log/ErrorMessage.hpp"

namespace Cool {

class Exception : public std::exception {
public:
    explicit Exception(ErrorMessage);
    auto what() const noexcept -> const char* override;
    auto error_message() const -> ErrorMessage const& { return _error_message; }

private:
    ErrorMessage _error_message;
};

} // namespace Cool