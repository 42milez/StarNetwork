#ifndef P2P_TECHDEMO_RUDPEVENT_H
#define P2P_TECHDEMO_RUDPEVENT_H

#include "RUdpEventType.h"
#include "RUdpPeer.h"

using UdpEvent = struct UdpEvent
{
    UdpEventType type;

    uint32_t data;

    std::shared_ptr<RUdpPeer> peer;

    std::shared_ptr<RUdpPacket> packet;

    uint8_t channel_id;

    UdpEvent();
};

#endif // P2P_TECHDEMO_RUDPEVENT_H
