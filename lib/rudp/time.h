#ifndef STAR_NETWORK_LIB_RUDP_TIME_H_
#define STAR_NETWORK_LIB_RUDP_TIME_H_

#include <cstdint>

#include "lib/core/os.h"
#include "lib/core/singleton.h"

namespace rudp
{
    class Time
    {
      public:
        Time() = delete;

        ~Time() = delete;

        Time(const Time &) = delete;

        Time(Time &&) = delete;

        Time &
        operator=(const Time &) = delete;

        Time &
        operator=(Time &&) = delete;

      public:
        static uint32_t
        Get();

        static void
        Set(uint32_t new_time_base);

      private:
        static uint32_t time_base_;
    };
} // namespace rudp

#endif // STAR_NETWORK_LIB_RUDP_TIME_H_
