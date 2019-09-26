#include "RUdpTime.h"

uint32_t RUdpTime::time_base_ = 0;

uint32_t
RUdpTime::Get()
{
    return Singleton<OS>::Instance().get_ticks_msec() - time_base_;
}

void
RUdpTime::Set(uint32_t new_time_base)
{
    time_base_ = Singleton<OS>::Instance().get_ticks_msec() - new_time_base;
}
