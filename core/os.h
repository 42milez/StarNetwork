#ifndef P2P_TECHDEMO_CORE_OS_H
#define P2P_TECHDEMO_CORE_OS_H

#include <cstdint>

class OS
{
private:
    uint64_t _clock_start = 0;
    double _clock_scale = 0;
public:
    uint64_t get_ticks_usec() const;
    uint32_t get_ticks_msec() const;
};

#endif // P2P_TECHDEMO_CORE_OS_H
