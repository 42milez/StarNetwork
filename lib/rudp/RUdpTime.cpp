#include "RUdpTime.h"

uint32_t RUdpTime::time_base_ = 0;

uint32_t
RUdpTime::Get()
{
    return core::Singleton<OS>::Instance().GetTicksMsec() - time_base_;
}

void
RUdpTime::Set(uint32_t new_time_base)
{
    time_base_ = core::Singleton<OS>::Instance().GetTicksMsec() - new_time_base;
}
