#include "RUdpAddress.h"

#include <cstring>

RUdpAddress::RUdpAddress() : port(0), wildcard(0)
{
    memset(&host, 0, sizeof(host));
}

void
RUdpAddress::set_ip(const uint8_t *ip, size_t size)
{
    auto len = size > 16 ? 16 : size;
    memset(host, 0, 16);
    memcpy(host, ip, len); // network byte-order (big endian)
}
