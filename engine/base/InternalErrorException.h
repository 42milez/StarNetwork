#ifndef LIFE_INTERNAL_ERROR_EXCEPTION_H
#define LIFE_INTERNAL_ERROR_EXCEPTION_H

#include <string>

namespace engine
{
  namespace base
  {
    class InternalErrorException {
    public:
      explicit InternalErrorException(std::string msg);

      const char *what() const noexcept;

    private:
      std::string msg_;
    };
  } // namespace util
}

#endif // LIFE_INTERNAL_ERROR_EXCEPTION_H
