#ifndef P2P_TECHDEMO_RUDPPACKETFLAG_H
#define P2P_TECHDEMO_RUDPPACKETFLAG_H

#include <cstdint>

enum class UdpPacketFlag : uint32_t
{
    // packet must be received by the target peer and
    // resend attempts should be made until the packet is delivered
        RELIABLE = (1u << 0u),

    // packet will not be sequenced with other packets not supported for reliable packets
        UNSEQUENCED = (1u << 1u),

    // packet will not allocate data, and user must supply it instead
        NO_ALLOCATE = (1u << 2u),

    // packet will be fragmented using unreliable (instead of reliable) sends if it exceeds the MTU
        UNRELIABLE_FRAGMENT = (1u << 3u),

    // whether the packet has been sent from all queues it has been entered into
        SENT = (1u << 8u)
};

#endif // P2P_TECHDEMO_RUDPPACKETFLAG_H
