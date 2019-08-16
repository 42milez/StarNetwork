#ifndef P2P_TECHDEMO_RUDPDISPATCHHUB_H
#define P2P_TECHDEMO_RUDPDISPATCHHUB_H

#include "RUdpDispatchQueue.h"
#include "RUdpEvent.h"

class RUdpDispatchHub
{
public:
    RUdpDispatchHub();

    void Connect(const std::shared_ptr<RUdpPeer> &peer);
    void Disconnect(const std::shared_ptr<RUdpPeer> &peer);

    void ChangeState(const std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState &state);

    void NotifyConnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event);

public:
    inline void Enqueue(std::shared_ptr<RUdpPeer> &peer)
    { queue_->Enqueue(peer); }

    inline std::shared_ptr<RUdpPeer> Dequeue()
    { return queue_->Dequeue(); }

    void DispatchState(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state);

    inline bool PeerExists()
    { return queue_->PeerExists(); }

public:
    inline size_t bandwidth_limited_peers()
    { return bandwidth_limited_peers_; }

    inline size_t connected_peers()
    { return connected_peers_; }

    inline bool recalculate_bandwidth_limits()
    { return recalculate_bandwidth_limits_; }

    inline void recalculate_bandwidth_limits(bool val)
    { recalculate_bandwidth_limits_ = val; }

private:
    std::unique_ptr<RUdpDispatchQueue> queue_;

    size_t bandwidth_limited_peers_;
    size_t connected_peers_;

    bool recalculate_bandwidth_limits_;
};

#endif // P2P_TECHDEMO_RUDPDISPATCHHUB_H