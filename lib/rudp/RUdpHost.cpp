#include "core/error_macros.h"
#include "core/hash.h"

#include "RUdpCommon.h"
#include "RUdpEvent.h"
#include "RUdpHost.h"
#include "RUdpPeerPod.h"

RUdpHost::RUdpHost(const RUdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth,
                   uint32_t out_bandwidth)
    :
    channel_count_(channel_count),
    duplicate_peers_(PROTOCOL_MAXIMUM_PEER_ID),
    incoming_bandwidth_(in_bandwidth),
    maximum_segment_size_(HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE),
    maximum_waiting_data_(HOST_DEFAULT_MAXIMUM_WAITING_DATA),
    mtu_(HOST_DEFAULT_MTU),
    outgoing_bandwidth_(out_bandwidth),
    received_address_(std::make_unique<RUdpAddress>()),
    received_data_length_(),
    segment_data_(),
    service_time_()
{
    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID) {
        // TODO: throw exception
        // ...
    }

    _conn = std::make_shared<RUdpConnection>(address);

    peer_pod_ = std::make_unique<RUdpPeerPod>(peer_count, _conn);
}

Error
RUdpHost::Connect(const RUdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto peer = peer_pod_->AvailablePeerExists();

    if (peer == nullptr)
        return Error::CANT_CREATE;

    auto err = peer->Setup(address, channel_count, data, incoming_bandwidth_, outgoing_bandwidth_);

    return err;
}

#define CHECK_RETURN_VALUE(val)                \
    if (val == EventStatus::AN_EVENT_OCCURRED) \
        return EventStatus::AN_EVENT_OCCURRED; \
    else if (val == EventStatus::ERROR)        \
        return EventStatus::ERROR;

/** Waits for events on the host specified and shuttles packets between
    the host and its peers.

    @param event   an event structure where event details will be placed if one occurs
                   if event == nullptr then no events will be delivered
    @param timeout number of milliseconds that RUDP should wait for events

    @retval > 0    if an event occurred within the specified time limit
    @retval 0      if no event occurred
    @retval < 0    on failure

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

    service_time_ = TimeGet();

    timeout += service_time_;

    uint8_t wait_condition;

    do {
        peer_pod_->BandwidthThrottle(service_time_, incoming_bandwidth_, outgoing_bandwidth_);

        ret = peer_pod_->SendOutgoingCommands(event, service_time_, true);

        CHECK_RETURN_VALUE(ret)

        //ret = protocol_receive_incoming_commands(host, event);

        CHECK_RETURN_VALUE(ret)

        //ret = protocol_send_outgoing_commands(host, event, 1);

        CHECK_RETURN_VALUE(ret)

        //ret = _udp_protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)

        if (UDP_TIME_GREATER_EQUAL(service_time_, timeout))
            return EventStatus::NO_EVENT_OCCURRED;

        do {
            service_time_ = TimeGet();

            if (UDP_TIME_GREATER_EQUAL(service_time_, timeout))
                return EventStatus::NO_EVENT_OCCURRED;

            wait_condition =
                static_cast<uint8_t>(SocketWait::RECEIVE) | static_cast<uint8_t>(SocketWait::INTERRUPT);

            if (SocketWait(wait_condition, UDP_TIME_DIFFERENCE(timeout, service_time_)) != 0)
                return EventStatus::ERROR;
        }
        while (wait_condition & static_cast<uint32_t>(SocketWait::INTERRUPT));

        service_time_ = TimeGet();
    }
    while (wait_condition & static_cast<uint32_t>(SocketWait::RECEIVE));

    return EventStatus::NO_EVENT_OCCURRED;
}
