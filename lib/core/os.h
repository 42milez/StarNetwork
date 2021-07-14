#ifndef STAR_NETWORK_LIB_CORE_OS_H_
#define STAR_NETWORK_LIB_CORE_OS_H_

#include <cstdint>
#include <string>

namespace core
{
    class OS
    {
      public:
        OS();

        [[nodiscard]] uint64_t
        GetTicksUsec() const;

        inline uint32_t
        GetTicksMsec()
        {
            return GetTicksUsec() / 1000;
        };

        static inline uint64_t
        GetUnixTime()
        {
            return 0;
        };

        static inline std::string
        GetUserDataDir()
        {
            return ".";
        };

      private:
        uint64_t clock_start_;
    };
} // namespace core

#endif // STAR_NETWORK_LIB_CORE_OS_H_
