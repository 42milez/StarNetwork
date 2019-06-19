#include <arpa/inet.h>

#include "core/os.h"
#include "core/singleton.h"

#include "RUdpCommon.h"

static uint32_t time_base;

uint32_t
udp_time_get()
{
    return Singleton<OS>::Instance().get_ticks_msec() - time_base;
}

void
udp_time_set(uint32_t new_time_base)
{
    time_base = Singleton<OS>::Instance().get_ticks_msec() - new_time_base;
}
