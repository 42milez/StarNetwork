#ifndef P2P_TECHDEMO_RUDPADDRESS_H
#define P2P_TECHDEMO_RUDPADDRESS_H

#include <cstdint>
#include <cstring>

using RUdpAddress = struct RUdpAddress
{
public:
    RUdpAddress();

    void SetIP(const uint8_t *ip, size_t size);

public:
    RUdpAddress & operator =(const RUdpAddress &address);
    bool operator ==(const RUdpAddress &address) const;
    bool operator !=(const RUdpAddress &address) const;

public:
    uint8_t host[16];

    uint16_t port;

    uint8_t wildcard;
};

#endif // P2P_TECHDEMO_RUDPADDRESS_H
