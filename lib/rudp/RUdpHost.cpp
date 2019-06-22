#include "core/error_macros.h"
#include "core/hash.h"

#include "RUdpCommon.h"
#include "RUdpEvent.h"
#include "RUdpHost.h"
#include "RUdpPeerPod.h"

RUdpHost::RUdpHost(const RUdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth,
                   uint32_t out_bandwidth) :
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
    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID)
    {
        // TODO: throw exception
        // ...
    }

    _conn = std::make_shared<RUdpConnection>(address);

    peer_pod_ = std::make_unique<RUdpPeerPod>(peer_count);
}

Error
RUdpHost::Connect(const RUdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto peer = peer_pod_->available_peer_exists();

    if (peer == nullptr)
        return Error::CANT_CREATE;

    auto err = peer->setup(address, channel_count, data, incoming_bandwidth_, outgoing_bandwidth_);

    return err;
}

int
RUdpHost::Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
{
#define CHECK_RETURN_VALUE(val) \
    if (val == 1)               \
        return 1;               \
    else if (val == -1)         \
        return -1;

    int ret;

    if (event != nullptr)
    {
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
        ret = peer_pod_->protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)
    }

    service_time_ = udp_time_get();

    timeout += service_time_;

    uint32_t wait_condition;

    do
    {
        // 帯域幅の調整
        //if (UDP_TIME_DIFFERENCE(_service_time, _bandwidth_throttle_epoch) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
        //    _udp_host_bandwidth_throttle();

        peer_pod_->protocol_bandwidth_throttle(service_time_, incoming_bandwidth_, outgoing_bandwidth_);

        //
        ret = peer_pod_->send_outgoing_commands(event, service_time_, true);

        CHECK_RETURN_VALUE(ret)

        //ret = protocol_receive_incoming_commands(host, event);

        CHECK_RETURN_VALUE(ret)

        //ret = protocol_send_outgoing_commands(host, event, 1);

        CHECK_RETURN_VALUE(ret)

        //ret = _udp_protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)

        if (UDP_TIME_GREATER_EQUAL(service_time_, timeout))
            return 0;

        do
        {
            service_time_ = udp_time_get();

            if (UDP_TIME_GREATER_EQUAL(service_time_, timeout))
                return 0;

            wait_condition =
                static_cast<uint32_t>(SocketWait::RECEIVE) | static_cast<uint32_t>(SocketWait::INTERRUPT);
        } while (wait_condition & static_cast<uint32_t>(SocketWait::INTERRUPT));

        service_time_ = udp_time_get();
    } while (wait_condition & static_cast<uint32_t>(SocketWait::RECEIVE));

    return 0;
}
