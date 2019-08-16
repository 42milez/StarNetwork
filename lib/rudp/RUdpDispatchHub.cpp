#include "RUdpDispatchHub.h"

RUdpDispatchHub::RUdpDispatchHub() :
    bandwidth_limited_peers_(),
    connected_peers_(),
    dispatch_queue_(std::make_unique<RUdpDispatchQueue>()),
    recalculate_bandwidth_limits_(false)
{}

void
RUdpDispatchHub::ChangeState(const std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState &state,
                             size_t &bandwidth_limited_peers, size_t &connected_peers)
{
    if (state == RUdpPeerState::CONNECTED || state == RUdpPeerState::DISCONNECT_LATER)
    {
        peer->Connect(bandwidth_limited_peers_, connected_peers_);
    }
    else
    {
        peer->Disconnect(bandwidth_limited_peers_, connected_peers_);
    }

    peer->net()->state(state);
}

void
RUdpDispatchHub::DispatchState(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state)
{
    ChangeState(peer, state, bandwidth_limited_peers_, connected_peers_);

    if (!peer->needs_dispatch()) {
        peer->needs_dispatch(true);

        dispatch_queue_->Enqueue(peer);
    }
}

void
RUdpDispatchHub::NotifyConnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event)
{
    recalculate_bandwidth_limits_ = true;

    if (event)
    {
        ChangeState(RUdpPeerState::CONNECTED);

        event->type = RUdpEventType::CONNECT;
        event->peer = peer;
        event->data = peer->event_data();
    }
    else
    {
        DispatchState(peer, peer->StateIs(RUdpPeerState::CONNECTING) ?
                                RUdpPeerState::CONNECTION_SUCCEEDED :
                                RUdpPeerState::CONNECTION_PENDING);
    }
}
