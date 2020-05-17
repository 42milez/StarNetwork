#include <string>

#include "initialization_exception.h"

namespace core
{
    InitializationException::InitializationException(std::string &&msg)
        : _msg(std::move(msg))
    {
    }

    const char *
    InitializationException::what() const noexcept
    {
        return _msg.c_str();
    }
} // namespace core
