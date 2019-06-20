#ifndef P2P_TECHDEMO_RUDPEVENT_H
#define P2P_TECHDEMO_RUDPEVENT_H

#include "RUdpPeer.h"

using RUdpEvent = struct RUdpEvent
{
    enum class RUdpEventType : int
    {
        NONE,
        CONNECT,
        DISCONNECT,
        RECEIVE
    };

    RUdpEventType type;

    uint32_t data;

    std::shared_ptr<RUdpPeer> peer;

    std::shared_ptr<RUdpPacket> packet;

    uint8_t channel_id;

    RUdpEvent();
};

#endif // P2P_TECHDEMO_RUDPEVENT_H
