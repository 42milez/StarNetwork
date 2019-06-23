#ifndef P2P_TECHDEMO_RUDPSEGMENTFLAG_H
#define P2P_TECHDEMO_RUDPSEGMENTFLAG_H

#include <cstdint>

enum class RUdpSegmentFlag: uint32_t
{
    // segment must be received by the target peer and
    // resend attempts should be made until the segment is delivered
        RELIABLE = (1u << 0u),

    // segment will not be sequenced with other segments not supported for reliable segments
        UNSEQUENCED = (1u << 1u),

    // segment will not allocate data, and user must supply it instead
        NO_ALLOCATE = (1u << 2u),

    // segment will be fragmented using unreliable (instead of reliable) sends if it exceeds the MTU
        UNRELIABLE_FRAGMENT = (1u << 3u),

    // whether the segment has been sent from all queues it has been entered into
        SENT = (1u << 8u)
};

#endif // P2P_TECHDEMO_RUDPSEGMENTFLAG_H
