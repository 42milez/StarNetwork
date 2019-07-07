#include "RUdpAddress.h"

#include <cstring>

RUdpAddress::RUdpAddress()
    : port(), wildcard(false)
{
    memset(&host, 0, sizeof(host));
}

void
RUdpAddress::SetIP(const uint8_t *ip, size_t size)
{
    auto len = size > 16 ? 16 : size;

    memset(host, 0, 16);

    memcpy(host, ip, len); // network byte-order (big endian)
}

RUdpAddress &
RUdpAddress::operator=(const RUdpAddress &address)
{
    memcpy(host, address.host, 16);
    port = address.port;
}

bool
RUdpAddress::operator==(const RUdpAddress &address)
{
    auto same_host = memcmp(address.host, address.host, 16) == 0;
    auto same_port = port == address.port;

    return same_host & same_port;
}

bool
RUdpAddress::operator!=(const RUdpAddress &address)
{
    auto same_host = memcmp(address.host, address.host, 16) == 0;
    auto same_port = port == address.port;

    return !(same_host & same_port);
}
