#ifndef P2P_TECHDEMO_RUDPHOST_H
#define P2P_TECHDEMO_RUDPHOST_H

#include <functional>
#include <vector>

#include "lib/rudp/peer/RUdpPeerPod.h"
#include "RUdpAddress.h"
#include "RUdpConnection.h"

class RUdpHost
{
public:
    RUdpHost(const RUdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth,
             uint32_t out_bandwidth);

    Error
    Connect(const RUdpAddress &address, SysCh channel_count, uint32_t data);

    void
    RequestPeerRemoval(uint32_t peer_idx, const std::shared_ptr<RUdpPeer> &peer);

    EventStatus
    Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout);

    inline Error
    DisconnectNow(const std::shared_ptr<RUdpPeer> &peer, uint32_t data) { return peer_pod_->DisconnectNow(peer, data); }

    inline Error
    DisconnectLater(const std::shared_ptr<RUdpPeer> &peer, uint32_t data) { return peer_pod_->DisconnectLater(peer, data); }

    inline RUdpPeerState
    PeerState(size_t idx) { return peer_pod_->Peer(idx)->net()->state(); }

private:
    inline int
    SocketWait(uint8_t wait_condition, uint32_t timeout) { return 0; };

private:
    std::shared_ptr<RUdpConnection> conn_;
    std::unique_ptr<RUdpPeerPod> peer_pod_;

    SysCh channel_count_;

    size_t maximum_segment_size_;
    size_t maximum_waiting_data_;

    uint32_t incoming_bandwidth_;
    uint32_t outgoing_bandwidth_;
    uint32_t mtu_;
};

#endif // P2P_TECHDEMO_RUDPHOST_H
