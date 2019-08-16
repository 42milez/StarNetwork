#ifndef P2P_TECHDEMO_RUDPDISPATCHHUB_H
#define P2P_TECHDEMO_RUDPDISPATCHHUB_H

#include "RUdpDispatchQueue.h"
#include "RUdpEvent.h"

class RUdpDispatchHub
{
public:
    RUdpDispatchHub();

    void NotifyConnect(std::shared_ptr<RUdpPeer> &peer, std::unique_ptr<RUdpEvent> &event);

public:
    inline void Enqueue(std::shared_ptr<RUdpPeer> &peer)
    { dispatch_queue_->Enqueue(peer); }

    inline std::shared_ptr<RUdpPeer> Dequeue()
    { return dispatch_queue_->Dequeue(); }

    void DispatchState(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state);

    inline bool PeerExists()
    { return dispatch_queue_->PeerExists(); }

public:
    inline bool recalculate_bandwidth_limits()
    { return recalculate_bandwidth_limits_; }

    inline void recalculate_bandwidth_limits(bool val)
    { recalculate_bandwidth_limits_ = val; }

private:
    std::unique_ptr<RUdpDispatchQueue> dispatch_queue_;

    bool recalculate_bandwidth_limits_;
};

#endif // P2P_TECHDEMO_RUDPDISPATCHHUB_H
