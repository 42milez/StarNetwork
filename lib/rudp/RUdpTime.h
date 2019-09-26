#ifndef P2P_TECHDEMO_RUDPTIME_H
#define P2P_TECHDEMO_RUDPTIME_H

#include <cstdint>

#include "core/os.h"
#include "core/singleton.h"

class RUdpTime
{
public:
    RUdpTime() = delete;
    ~RUdpTime() = delete;
    RUdpTime(const RUdpTime&) = delete;
    RUdpTime& operator=(const RUdpTime&) = delete;
    RUdpTime(RUdpTime&&) = delete;
    RUdpTime& operator=(RUdpTime&&) = delete;

public:
    static uint32_t Get();
    static void Set(uint32_t new_time_base);

private:
    static uint32_t time_base_;
};

#endif // P2P_TECHDEMO_RUDPTIME_H
