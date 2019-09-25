#include "core/os.h"
#include "core/singleton.h"

static uint32_t time_base;

uint32_t
TimeGet()
{
    return Singleton<OS>::Instance().get_ticks_msec() - time_base;
}

void
TimeSet(uint32_t new_time_base)
{
    time_base = Singleton<OS>::Instance().get_ticks_msec() - new_time_base;
}
