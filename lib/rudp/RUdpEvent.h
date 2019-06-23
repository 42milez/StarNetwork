#ifndef P2P_TECHDEMO_RUDPEVENT_H
#define P2P_TECHDEMO_RUDPEVENT_H

#include "RUdpPeer.h"

enum class RUdpEventType: int
{
    NONE,
    CONNECT,
    DISCONNECT,
    RECEIVE
};

using RUdpEvent = struct RUdpEvent
{
public:
    RUdpEvent();

public:
    RUdpEventType type;

    std::shared_ptr<RUdpPeer> peer;
    std::shared_ptr<RUdpSegment> segment;

    uint32_t data;

    uint8_t channel_id;
};

#endif // P2P_TECHDEMO_RUDPEVENT_H
