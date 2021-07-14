#ifndef STAR_NETWORK_LIB_RUDP_ENUM_H_
#define STAR_NETWORK_LIB_RUDP_ENUM_H_

#include <cstdint>

namespace rudp
{
    enum class EventStatus : int
    {
        AN_EVENT_OCCURRED = 1,
        NO_EVENT_OCCURRED = 0,
        ERROR             = -1
    };

    enum class EventType : uint8_t
    {
        NONE,
        CONNECT,
        DISCONNECT,
        RECEIVE,
        RECEIVE_ACK
    };

    enum class RUdpPeerState : uint8_t
    {
        DISCONNECTED = 0,
        CONNECTING,
        ACKNOWLEDGING_CONNECT,
        CONNECTION_PENDING,
        CONNECTION_SUCCEEDED,
        CONNECTED,
        DISCONNECT_LATER,
        DISCONNECTING,
        ACKNOWLEDGING_DISCONNECT,
        ZOMBIE
    };

    enum class RUdpProtocolCommand : uint8_t
    {
        NONE                     = 0,
        ACKNOWLEDGE              = 1,
        CONNECT                  = 2,
        VERIFY_CONNECT           = 3,
        DISCONNECT               = 4,
        PING                     = 5,
        SEND_RELIABLE            = 6,
        SEND_UNRELIABLE          = 7,
        SEND_FRAGMENT            = 8,
        SEND_UNSEQUENCED         = 9,
        BANDWIDTH_LIMIT          = 10,
        THROTTLE_CONFIGURE       = 11,
        SEND_UNRELIABLE_FRAGMENT = 12,
        COUNT                    = 13
    };

    enum class RUdpProtocolFlag : uint16_t
    {
        COMMAND_ACKNOWLEDGE  = (1u << 7u),
        COMMAND_UNSEQUENCED  = (1u << 6u),
        HEADER_COMPRESSED    = (1u << 14u),
        HEADER_SENT_TIME     = (1u << 15u),
        HEADER_MASK          = HEADER_COMPRESSED | HEADER_SENT_TIME,
        HEADER_SESSION_MASK  = (3u << 12u),
        HEADER_SESSION_SHIFT = 12
    };

    enum class SegmentFlag : uint16_t
    {
        // segment must be received by the target peer and
        // resend attempts should be made until the segment is delivered
        RELIABLE = (1u << 0u),

        // segment will not be sequenced with other segments not supported for reliable segments
        UNSEQUENCED = (1u << 1u),

        // segment will not allocate, and user must supply it instead
        NO_ALLOCATE = (1u << 2u),

        // segment will be fragmented using unreliable (instead of reliable) sends if it exceeds the MTU
        UNRELIABLE_FRAGMENT = (1u << 3u),

        // whether the segment has been sent from all queues it has been entered into
        SENT = (1u << 8u)
    };

    enum class SocketWait : uint8_t
    {
        NONE      = 0,
        SEND      = (1u << 0u),
        RECEIVE   = (1u << 1u),
        INTERRUPT = (1u << 2u)
    };
} // namespace rudp

#endif // STAR_NETWORK_LIB_RUDP_ENUM_H_
