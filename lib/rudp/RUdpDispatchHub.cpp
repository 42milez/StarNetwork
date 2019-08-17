#include "RUdpDispatchHub.h"

RUdpDispatchHub::RUdpDispatchHub() :
    bandwidth_limited_peers_(),
    connected_peers_(),
    queue_(std::make_unique<RUdpDispatchQueue>()),
    recalculate_bandwidth_limits_(false)
{}

void
RUdpDispatchHub::Connect(const std::shared_ptr<RUdpPeer> &peer)
{
    if (!peer->net()->StateIs(RUdpPeerState::CONNECTED) && !peer->net()->StateIs(RUdpPeerState::DISCONNECT_LATER))
    {
        if (peer->net()->incoming_bandwidth() != 0)
            ++bandwidth_limited_peers_;

        ++connected_peers_;
    }
}

void
RUdpDispatchHub::Disconnect(const std::shared_ptr<RUdpPeer> &peer)
{
    if (peer->net()->StateIs(RUdpPeerState::CONNECTED) || peer->net()->StateIs(RUdpPeerState::DISCONNECT_LATER))
    {
        if (peer->net()->incoming_bandwidth() != 0)
            --bandwidth_limited_peers_;

        --connected_peers_;
    }
}

void
RUdpDispatchHub::ChangeState(const std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState &state)
{
    if (state == RUdpPeerState::CONNECTED || state == RUdpPeerState::DISCONNECT_LATER)
    {
        Connect(peer);
    }
    else
    {
        Disconnect(peer);
    }

    peer->net()->state(state);
}

void
RUdpDispatchHub::DispatchState(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state)
{
    ChangeState(peer, state);

    if (!peer->needs_dispatch()) {
        peer->needs_dispatch(true);

        queue_->Enqueue(peer);
    }
}

void
RUdpDispatchHub::NotifyConnect(const std::unique_ptr<RUdpEvent> &event, std::shared_ptr<RUdpPeer> &peer)
{
    recalculate_bandwidth_limits_ = true;

    if (event)
    {
        ChangeState(peer, RUdpPeerState::CONNECTED);

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
