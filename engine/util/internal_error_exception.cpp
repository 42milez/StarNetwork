#include <string>

#include "internal_error_exception.h"

namespace util
{
  InternalErrorException::InternalErrorException(std::string msg) : msg_(std::move(msg)) {}

  const char *InternalErrorException::what() const noexcept {
    return msg_.c_str();
  }
}
