#ifndef P2P_TECHDEMO_RUDPADDRESS_H
#define P2P_TECHDEMO_RUDPADDRESS_H

#include <cstdint>
#include <cstring>

using RUdpAddress = struct RUdpAddress
{
    uint8_t host[16] = {0};

    uint16_t port = 0;

    uint8_t wildcard = 0;

    RUdpAddress();

    void set_ip(const uint8_t *ip, size_t size);
};

#endif // P2P_TECHDEMO_RUDPADDRESS_H
