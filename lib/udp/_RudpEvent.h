#ifndef P2P_TECHDEMO_RUDPEVENT_H
#define P2P_TECHDEMO_RUDPEVENT_H

using UdpEvent = struct UdpEvent
{
    UdpEventType type;

    uint32_t data;

    std::shared_ptr<UdpPeer> peer;

    std::shared_ptr<UdpPacket> packet;

    uint8_t channel_id;

    UdpEvent();
};

#endif // P2P_TECHDEMO_RUDPEVENT_H
