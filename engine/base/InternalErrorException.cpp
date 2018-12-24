#include <string>

#include "InternalErrorException.h"

namespace engine
{
  namespace base
  {
    InternalErrorException::InternalErrorException(std::string msg) : msg_(std::move(msg)) {}

    const char *InternalErrorException::what() const noexcept {
      return msg_.c_str();
    }
  }
}
