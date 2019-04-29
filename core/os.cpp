#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#include "os.h"

uint64_t
OS::get_ticks_usec() const
{
#ifdef __APPLE__
    uint64_t longtime = mach_absolute_time() * _clock_scale;
#else
    struct timespec tv_now = {0, 0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &tv_now);
    uint64_t longtime = ((uint64_t)tv_now.tv_nsec / 1000L) + (uint64_t)tv_now.tv_sec * 1000000L;
#endif
    longtime -= _clock_start;

    return longtime;
}

uint32_t
OS::get_ticks_msec() const
{
    return get_ticks_usec() / 1000;
}
