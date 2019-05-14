#ifndef P2P_TECHDEMO_UDP_PACKET_H
#define P2P_TECHDEMO_UDP_PACKET_H

#include <cstddef>
#include <cstdint>

using UdpPacket = struct UdpPacket
{
    size_t reference_count;
    uint32_t flags;
    uint8_t *data;
    size_t data_length;
    void *user_data;
};

#endif // P2P_TECHDEMO_UDP_PACKET_H
