#include "RUdpDispatchHub.h"

namespace
{
    void _dispatch_state(const std::unique_ptr<RUdpDispatchQueue> &queue, std::shared_ptr<RUdpPeer> &peer,
                         RUdpPeerState state)
    {
        peer->ChangeState(peer, state);

        if (!peer->needs_dispatch()) {
            peer->needs_dispatch(true);

            queue->Enqueue(peer);
        }
    }
}

RUdpDispatchHub::RUdpDispatchHub() :
    dispatch_queue_(std::make_unique<RUdpDispatchQueue>()),
    recalculate_bandwidth_limits_(false)
{}

void
RUdpDispatchHub::DispatchState(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state)
{
    _dispatch_state(dispatch_queue_, peer, state);
}

void
RUdpDispatchHub::NotifyConnect(std::shared_ptr<RUdpPeer> &peer, std::unique_ptr<RUdpEvent> &event)
{
    recalculate_bandwidth_limits_ = true;

    if (event)
    {
        peer->ChangeState(RUdpPeerState::CONNECTED);

        event->type = RUdpEventType::CONNECT;
        event->peer = peer;
        event->data = peer->event_data();
    }
    else
    {
        _dispatch_state(dispatch_queue_, peer, peer->StateIs(RUdpPeerState::CONNECTING) ?
                                                   RUdpPeerState::CONNECTION_SUCCEEDED :
                                                   RUdpPeerState::CONNECTION_PENDING);
    }
}
