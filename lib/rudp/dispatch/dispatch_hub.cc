#include "lib/rudp/dispatch/dispatch_hub.h"

namespace rudp
{
    DispatchHub::DispatchHub()
        : bandwidth_limited_peers_()
        , connected_peers_()
        , queue_(std::make_unique<DispatchQueue>())
        , recalculate_bandwidth_limits_()
    {
    }

    void
    DispatchHub::MergePeer(const std::shared_ptr<Peer> &peer)
    {
        auto &net = peer->net();

        if (net->StateIsNot(RUdpPeerState::CONNECTED) && net->StateIsNot(RUdpPeerState::DISCONNECT_LATER)) {
            if (net->incoming_bandwidth() != 0)
                ++bandwidth_limited_peers_;

            ++connected_peers_;
        }
    }

    void
    DispatchHub::PurgePeer(const std::shared_ptr<Peer> &peer)
    {
        auto &net = peer->net();

        if (net->StateIs(RUdpPeerState::CONNECTED) || net->StateIs(RUdpPeerState::DISCONNECT_LATER)) {
            if (net->incoming_bandwidth() != 0)
                --bandwidth_limited_peers_;

            --connected_peers_;
        }
    }

    void
    DispatchHub::ChangeState(const std::shared_ptr<Peer> &peer, const RUdpPeerState &state)
    {
        if (state == RUdpPeerState::CONNECTED || state == RUdpPeerState::DISCONNECT_LATER)
            MergePeer(peer);
        else
            PurgePeer(peer);

        peer->net()->state(state);
    }

    void
    DispatchHub::DispatchState(std::shared_ptr<Peer> &peer, RUdpPeerState state)
    {
        ChangeState(peer, state);

        if (!peer->needs_dispatch()) {
            peer->needs_dispatch(true);

            queue_->Enqueue(peer);
        }
    }

    void
    DispatchHub::NotifyConnect(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer)
    {
        recalculate_bandwidth_limits(true);

        if (event) {
            ChangeState(peer, RUdpPeerState::CONNECTED);

            event->type(EventType::CONNECT);
            event->peer(peer);
            event->data(peer->event_data());
        }
        else {
            DispatchState(peer, peer->net()->StateIs(RUdpPeerState::CONNECTING) ? RUdpPeerState::CONNECTION_SUCCEEDED
                                                                                : RUdpPeerState::CONNECTION_PENDING);
        }
    }

    void
    DispatchHub::NotifyDisconnect(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer)
    {
        auto &net = peer->net();

        if (net->StateIs(RUdpPeerState::CONNECTION_PENDING))
            recalculate_bandwidth_limits(true);

        if (net->StateIsNot(RUdpPeerState::CONNECTING) &&
            peer->StateAsNumber() < static_cast<uint8_t>(RUdpPeerState::CONNECTION_SUCCEEDED)) {
            peer->Reset();
        }
        else if (event) {
            event->type(EventType::DISCONNECT);
            event->peer(peer);
            event->data(0);

            peer->Reset();
        }
        else {
            peer->event_data(0);

            DispatchState(peer, RUdpPeerState::ZOMBIE);
        }
    }
} // namespace rudp
