#include "core/error_macros.h"
#include "core/hash.h"

#include "RUdpCommon.h"
#include "RUdpEvent.h"
#include "RUdpHost.h"
#include "RUdpPeerPod.h"

RUdpHost::RUdpHost(const RUdpAddress &address, SysCh channel_count, size_t peer_count,
                   uint32_t in_bandwidth, uint32_t out_bandwidth)
    : channel_count_(channel_count),
      conn_(std::make_shared<RUdpConnection>(address)),
      incoming_bandwidth_(in_bandwidth),
      maximum_segment_size_(HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE),
      maximum_waiting_data_(HOST_DEFAULT_MAXIMUM_WAITING_DATA),
      mtu_(HOST_DEFAULT_MTU),
      outgoing_bandwidth_(out_bandwidth)
{
    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID) {
        // TODO: throw exception
        // ...
    }

    peer_pod_ = std::make_unique<RUdpPeerPod>(peer_count, conn_, incoming_bandwidth_, outgoing_bandwidth_);
}

/** Initiates a connection to a foreign host.
 *
    @param address             destination for the connection
    @param channel_count       number of channels to allocate
    @param data                user data supplied to the receiving host

    @retval Error::CANT_CREATE Nothing available peer
    @retval Error::OK          A peer was successfully configured

    @remarks The peer configured will have not completed the connection until UdpHost::Service()
             notifies of an EventType::CONNECT event for the peer.
*/
Error
RUdpHost::Connect(const RUdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto peer = peer_pod_->AvailablePeer();

    if (peer == nullptr)
        return Error::CANT_CREATE;

    auto err = peer->Setup(address, channel_count, incoming_bandwidth_, outgoing_bandwidth_, data);

    return err;
}

#define CHECK_RETURN_VALUE(val)                \
    if (val == EventStatus::AN_EVENT_OCCURRED) \
        return EventStatus::AN_EVENT_OCCURRED; \
    else if (val == EventStatus::ERROR)        \
        return EventStatus::ERROR;

/** Waits for events on the host specified and shuttles packets between
    the host and its peers.

    @param event                           an event structure where event details will be placed if one occurs
                                           if event == nullptr then no events will be delivered
    @param timeout                         number of milliseconds that RUDP should wait for events

    @retval EventStatus::AN_EVENT_OCCURRED if an event occurred within the specified time limit
    @retval EventStatus::NO_EVENT_OCCURRED if no event occurred
    @retval EventStatus::ERROR             on failure

    @remarks RUdpHost::Service should be called fairly regularly for adequate performance
*/
EventStatus
RUdpHost::Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
{
    EventStatus ret;

    if (event != nullptr) {
        event->type = RUdpEventType::NONE;
        event->peer = nullptr;
        event->segment = nullptr;

        // - キューから取り出されたパケットは event に格納される
        // - ピアが取りうるステートは以下の 10 通りだが、この関数で処理されるのは 3,5,6,10 の４つ
        //  1. ACKNOWLEDGING_CONNECT,
        //  2. ACKNOWLEDGING_DISCONNECT,
        //  3. CONNECTED,
        //  4. CONNECTING,
        //  5. CONNECTION_PENDING,
        //  6. CONNECTION_SUCCEEDED,
        //  7. DISCONNECT_LATER,
        //  8. DISCONNECTED,
        //  9. DISCONNECTING,
        // 10. ZOMBIE
        ret = peer_pod_->DispatchIncomingCommands(event);

        CHECK_RETURN_VALUE(ret)
    }

    peer_pod_->update_service_time();

    timeout += peer_pod_->service_time();

    uint8_t wait_condition;

    do {
        peer_pod_->BandwidthThrottle(peer_pod_->service_time(), incoming_bandwidth_, outgoing_bandwidth_);

        ret = peer_pod_->SendOutgoingCommands(event, peer_pod_->service_time(), true);

        CHECK_RETURN_VALUE(ret)

        ret = peer_pod_->ReceiveIncomingCommands(event);

        CHECK_RETURN_VALUE(ret)

        ret = peer_pod_->SendOutgoingCommands(event, peer_pod_->service_time(), true);

        CHECK_RETURN_VALUE(ret)

        ret = peer_pod_->DispatchIncomingCommands(event);

        CHECK_RETURN_VALUE(ret)

        if (UDP_TIME_GREATER_EQUAL(peer_pod_->service_time(), timeout))
            return EventStatus::NO_EVENT_OCCURRED;

        do {
            peer_pod_->update_service_time();

            if (UDP_TIME_GREATER_EQUAL(peer_pod_->service_time(), timeout))
                return EventStatus::NO_EVENT_OCCURRED;

            wait_condition =
                static_cast<uint8_t>(SocketWait::RECEIVE) | static_cast<uint8_t>(SocketWait::INTERRUPT);

            if (SocketWait(wait_condition, UDP_TIME_DIFFERENCE(timeout, peer_pod_->service_time())) != 0)
                return EventStatus::ERROR;
        }
        while (wait_condition & static_cast<uint32_t>(SocketWait::INTERRUPT));

        peer_pod_->update_service_time();
    }
    while (wait_condition & static_cast<uint32_t>(SocketWait::RECEIVE));

    return EventStatus::NO_EVENT_OCCURRED;
}

Error
RUdpHost::Disconnect(const std::shared_ptr<RUdpPeer> &peer, uint32_t data)
{
    if (peer->StateIs(RUdpPeerState::DISCONNECTING) ||
        peer->StateIs(RUdpPeerState::DISCONNECTED) ||
        peer->StateIs(RUdpPeerState::ACKNOWLEDGING_DISCONNECT) ||
        peer->StateIs(RUdpPeerState::ZOMBIE))
    {
        return Error::ERROR;
    }

    peer->ResetPeerQueues();

    std::shared_ptr<RUdpProtocolType> cmd;

    cmd->header.command = PROTOCOL_COMMAND_DISCONNECT;
    cmd->header.channel_id = 0xFF;
    cmd->disconnect.data = htonl(data);

    if (peer->StateIs(RUdpPeerState::CONNECTED) || peer->StateIs(RUdpPeerState::DISCONNECT_LATER))
    {
        cmd->header.command |= PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    }
    else
    {
        cmd->header.command |= PROTOCOL_COMMAND_FLAG_UNSEQUENCED;
    }

    peer->QueueOutgoingCommand(cmd, nullptr, 0, 0);

    if (peer->StateIs(RUdpPeerState::CONNECTED) || peer->StateIs(RUdpPeerState::DISCONNECT_LATER))
    {
        peer_pod_->PeerOnDisconnect(peer);

        peer->net()->state(RUdpPeerState::DISCONNECTING);
    }
    else
    {
        peer_pod_->Flush();
        peer->Reset();
    }

    return Error::OK;
}

Error
RUdpHost::DisconnectNow(const std::shared_ptr<RUdpPeer> &peer, uint32_t data)
{
    if (peer->StateIs(RUdpPeerState::DISCONNECTED))
        return Error::ERROR;

    std::shared_ptr<RUdpProtocolType> cmd = std::make_shared<RUdpProtocolType>();

    if (!peer->StateIs(RUdpPeerState::ZOMBIE) && !peer->StateIs(RUdpPeerState::DISCONNECTING))
    {
        peer->ResetPeerQueues();

        cmd->header.command = PROTOCOL_COMMAND_DISCONNECT;
        cmd->header.channel_id = 0xFF;
        cmd->disconnect.data = htonl(data);

        peer->QueueOutgoingCommand(cmd, nullptr, 0, 0);

        peer_pod_->Flush();
    }

    peer->Reset();

    return Error::OK;
}

Error
RUdpHost::DisconnectLater(const std::shared_ptr<RUdpPeer> &peer, uint32_t data)
{
    if ((peer->StateIs(RUdpPeerState::CONNECTED) || peer->StateIs(RUdpPeerState::DISCONNECT_LATER)) &&
        (peer->command()->outgoing_reliable_command_exists() ||
         peer->command()->outgoing_unreliable_command_exists() ||
         peer->command()->sent_reliable_command_exists()))
    {
        peer->net()->state(RUdpPeerState::DISCONNECT_LATER);
        peer->event_data(data);
    }
    else
    {
        Disconnect(peer, data);
    }

    return Error::OK;
}

void
RUdpHost::RequestPeerRemoval(uint32_t peer_idx, const std::shared_ptr<RUdpPeer> &peer)
{
    peer_pod_->RequestPeerRemoval(peer_idx, peer);
}
