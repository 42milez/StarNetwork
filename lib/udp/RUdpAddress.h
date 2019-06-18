#ifndef P2P_TECHDEMO_RUDPADDRESS_H
#define P2P_TECHDEMO_RUDPADDRESS_H

#include <cstdint>

using UdpAddress = struct UdpAddress
{
    uint8_t host[16] = {0};

    uint16_t port = 0;

    uint8_t wildcard = 0;

    UdpAddress();
};

#endif // P2P_TECHDEMO_RUDPADDRESS_H
