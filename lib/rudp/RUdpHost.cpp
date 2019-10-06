#include "core/hash.h"

#include "lib/rudp/peer/RUdpPeerPod.h"
#include "RUdpMacro.h"
#include "RUdpEvent.h"
#include "RUdpHost.h"

RUdpHost::RUdpHost(const RUdpAddress &address, SysCh channel_count, size_t peer_count,
                   uint32_t in_bandwidth, uint32_t out_bandwidth)
    : channel_count_(channel_count),
      checksum_(),
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

void
RUdpHost::Broadcast(SysCh ch, std::shared_ptr<RUdpSegment> &segment)
{
    auto &peers = peer_pod_->peers();

    for (auto &peer : peers)
    {
        auto &net = peer->net();

        if (net->StateIsNot(RUdpPeerState::CONNECTED))
            continue;

        peer->Send(ch, segment, checksum_);
    }
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

#define RETURN_ON_EVENT_OCCURRED(val)       \
    if (val == EventStatus::AN_EVENT_OCCURRED) \
        return EventStatus::AN_EVENT_OCCURRED;

#define RETURN_ON_ERROR(val)       \
    if (val == EventStatus::ERROR) \
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
        event->Reset();

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

        RETURN_ON_EVENT_OCCURRED(ret)
        RETURN_ON_ERROR(ret)
    }

    peer_pod_->UpdateServiceTime();

    timeout += peer_pod_->service_time();

    uint8_t wait_condition;

    do {
        peer_pod_->BandwidthThrottle(peer_pod_->service_time(), incoming_bandwidth_, outgoing_bandwidth_);

        ret = peer_pod_->SendOutgoingCommands(event, peer_pod_->service_time(), true, checksum_);

        RETURN_ON_EVENT_OCCURRED(ret)
        RETURN_ON_ERROR(ret)

        ret = peer_pod_->ReceiveIncomingCommands(event, checksum_);

        RETURN_ON_EVENT_OCCURRED(ret)
        RETURN_ON_ERROR(ret)

        ret = peer_pod_->SendOutgoingCommands(event, peer_pod_->service_time(), true, checksum_);

        RETURN_ON_EVENT_OCCURRED(ret)
        RETURN_ON_ERROR(ret)

        ret = peer_pod_->DispatchIncomingCommands(event);

        RETURN_ON_EVENT_OCCURRED(ret)
        RETURN_ON_ERROR(ret)

        if (UDP_TIME_GREATER_EQUAL(peer_pod_->service_time(), timeout))
            return EventStatus::NO_EVENT_OCCURRED;

        do {
            peer_pod_->UpdateServiceTime();

            if (UDP_TIME_GREATER_EQUAL(peer_pod_->service_time(), timeout))
                return EventStatus::NO_EVENT_OCCURRED;

            wait_condition =
                static_cast<uint8_t>(SocketWait::RECEIVE) | static_cast<uint8_t>(SocketWait::INTERRUPT);

            if (SocketWait(wait_condition, UDP_TIME_DIFFERENCE(timeout, peer_pod_->service_time())) != 0)
                return EventStatus::ERROR;
        }
        while (wait_condition & static_cast<uint32_t>(SocketWait::INTERRUPT));

        peer_pod_->UpdateServiceTime();
    }
    while (wait_condition & static_cast<uint32_t>(SocketWait::RECEIVE));

    return EventStatus::NO_EVENT_OCCURRED;
}

void
RUdpHost::RequestPeerRemoval(uint32_t peer_idx, const std::shared_ptr<RUdpPeer> &peer)
{
    peer_pod_->RequestPeerRemoval(peer_idx, peer, checksum_);
}
