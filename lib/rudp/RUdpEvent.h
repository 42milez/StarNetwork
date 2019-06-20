#ifndef P2P_TECHDEMO_RUDPEVENT_H
#define P2P_TECHDEMO_RUDPEVENT_H

#include "RUdpPeer.h"

enum class RUdpEventType : int
{
    NONE,
    CONNECT,
    DISCONNECT,
    RECEIVE
};

using RUdpEvent = struct RUdpEvent
{
    RUdpEventType type;

    uint32_t data;

    std::shared_ptr<RUdpPeer> peer;

    std::shared_ptr<RUdpPacket> packet;

    uint8_t channel_id;

    RUdpEvent();
};

#endif // P2P_TECHDEMO_RUDPEVENT_H
