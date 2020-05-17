#include <string>

#include "lib/core/error_macros.h"
#include "os.h"

namespace core
{
    OS::OS()
    {
#if defined(CLOCK_MONOTONIC_RAW) && !defined(JAVASCRIPT_ENABLED) // This is a better clock on Linux.
#define RUDP_CLOCK CLOCK_MONOTONIC_RAW
#else
#define RUDP_CLOCK CLOCK_MONOTONIC
#endif
        struct timespec tv_now = {0, 0};
        // ERR_EXPLAIN("OS CLOCK IS NOT WORKING!");
        ERR_FAIL_COND(clock_gettime(RUDP_CLOCK, &tv_now) != 0);
        clock_start_           = ((uint64_t)tv_now.tv_nsec / 1000L) + (uint64_t)tv_now.tv_sec * 1000000L;
    }

    uint64_t
    OS::GetTicksUsec() const
    {
        struct timespec tv_now = {0, 0};
        clock_gettime(RUDP_CLOCK, &tv_now);
        uint64_t longtime = ((uint64_t)tv_now.tv_nsec / 1000L) + (uint64_t)tv_now.tv_sec * 1000000L;
        longtime -= clock_start_;

        return longtime;
    }
} // namespace core
