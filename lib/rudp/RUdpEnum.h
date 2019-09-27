#ifndef P2P_TECHDEMO_RUDPENUM_H
#define P2P_TECHDEMO_RUDPENUM_H

#include <cstdint>

enum class EventStatus: int
{
    AN_EVENT_OCCURRED = 1,
    NO_EVENT_OCCURRED = 0,
    ERROR = -1
};

enum class RUdpEventType: uint8_t
{
    NONE,
    CONNECT,
    DISCONNECT,
    RECEIVE
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

enum class SocketWait: uint8_t
{
    NONE = 0,
    SEND = (1u << 0u),
    RECEIVE = (1u << 1u),
    INTERRUPT = (1u << 2u)
};

enum class SysCh: uint8_t
{
    CONFIG,
    RELIABLE,
    UNRELIABLE,
    MAX
};

enum class SysMsg: uint8_t
{
    ADD_PEER,
    REMOVE_PEER
};

#endif // P2P_TECHDEMO_RUDPENUM_H
