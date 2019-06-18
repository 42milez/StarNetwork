#include <arpa/inet.h>

#include "core/os.h"
#include "core/singleton.h"

#include "Rudp.h"

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

void
udp_address_set_ip(UdpAddress &address, const uint8_t *ip, size_t size)
{
    auto len = size > 16 ? 16 : size;
    memset(address.host, 0, 16);
    memcpy(address.host, ip, len); // network byte-order (big endian)
}
