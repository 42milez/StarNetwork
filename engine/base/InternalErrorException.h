#ifndef P2P_TECHDEMO_ENGINE_BASE_INTERNAL_ERROR_EXCEPTION_H
#define P2P_TECHDEMO_ENGINE_BASE_INTERNAL_ERROR_EXCEPTION_H

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
  } // namespace base
} // namespace engine

#endif // P2P_TECHDEMO_ENGINE_BASE_INTERNAL_ERROR_EXCEPTION_H
