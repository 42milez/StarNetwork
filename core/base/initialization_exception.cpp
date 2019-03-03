#include <string>

#include "initialization_exception.h"

InitializationException::InitializationException(std::string &msg) : _msg(std::move(msg))
{}

const char *
InternalErrorException::what() const noexcept
{
    return _msg.c_str();
}
