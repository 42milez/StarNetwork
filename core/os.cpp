#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#include <string>

#include "os.h"

OS::OS()
{
#ifdef __APPLE__
    mach_timebase_info_data_t info;
    kern_return_t ret = mach_timebase_info(&info);
    //ERR_EXPLAIN("OS CLOCK IS NOT WORKING!");
    //ERR_FAIL_COND(ret != 0);
    clock_scale_ = (static_cast<double>(info.numer) / static_cast<double>(info.denom)) / 1000.0;
    clock_start_ = mach_absolute_time() * clock_scale_;
#else
#if defined(CLOCK_MONOTONIC_RAW) && !defined(JAVASCRIPT_ENABLED) // This is a better clock on Linux.
#define RUDP_CLOCK CLOCK_MONOTONIC_RAW
#else
#define RUDP_CLOCK CLOCK_MONOTONIC
#endif
  struct timespec tv_now = { 0, 0 };
  //ERR_EXPLAIN("OS CLOCK IS NOT WORKING!");
  //ERR_FAIL_COND(clock_gettime(RUDP_CLOCK, &tv_now) != 0);
  clock_start_ = ((uint64_t)tv_now.tv_nsec / 1000L) + (uint64_t)tv_now.tv_sec * 1000000L;
#endif
}

uint64_t
OS::GetTicksUsec() const
{
#ifdef __APPLE__
    uint64_t longtime = mach_absolute_time() * clock_scale_;
#else
    struct timespec tv_now = {0, 0};
    clock_gettime(RUDP_CLOCK, &tv_now);
    uint64_t longtime = ((uint64_t)tv_now.tv_nsec / 1000L) + (uint64_t)tv_now.tv_sec * 1000000L;
#endif
    longtime -= clock_start_;

    return longtime;
}
