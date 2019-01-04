#ifndef P2P_TECHDEMO_INTERNAL_ERROR_EXCEPTION_H
#define P2P_TECHDEMO_INTERNAL_ERROR_EXCEPTION_H

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

#endif // P2P_TECHDEMO_INTERNAL_ERROR_EXCEPTION_H
