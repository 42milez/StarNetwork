#include "core/error_macros.h"
#include "core/hash.h"

#include "RUdpCommon.h"
#include "RUdpEvent.h"
#include "RUdpHost.h"
#include "RUdpPeerPod.h"

namespace
{
    std::vector<size_t> command_sizes{
        0,
        sizeof(RUdpProtocolAcknowledge),
        sizeof(RUdpProtocolConnect),
        sizeof(RUdpProtocolVerifyConnect),
        sizeof(RUdpProtocolDisconnect),
        sizeof(RUdpProtocolPing),
        sizeof(RUdpProtocolSendReliable),
        sizeof(RUdpProtocolSendUnreliable),
        sizeof(RUdpProtocolSendFragment),
        sizeof(RUdpProtocolSendUnsequenced),
        sizeof(RUdpProtocolBandwidthLimit),
        sizeof(RUdpProtocolThrottleConfigure),
        sizeof(RUdpProtocolSendFragment)
    };
}

Error
RUdpHost::udp_host_connect(const UdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto peer = _peer_pod->available_peer_exists();

    if (peer == nullptr)
        return Error::CANT_CREATE;

    auto err = peer->setup(address, channel_count, data, _incoming_bandwidth, _outgoing_bandwidth);

    return err;
}

int
RUdpHost::udp_host_service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
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
        event->packet = nullptr;

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
        ret = _peer_pod->protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)
    }

    _service_time = udp_time_get();

    timeout += _service_time;

    uint32_t wait_condition;

    do
    {
        // 帯域幅の調整
        //if (UDP_TIME_DIFFERENCE(_service_time, _bandwidth_throttle_epoch) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
        //    _udp_host_bandwidth_throttle();

        _peer_pod->protocol_bandwidth_throttle(_service_time, _incoming_bandwidth, _outgoing_bandwidth);

        //
        ret = _peer_pod->send_outgoing_commands(event, _service_time, true);

        CHECK_RETURN_VALUE(ret)

        //ret = protocol_receive_incoming_commands(host, event);

        CHECK_RETURN_VALUE(ret)

        //ret = protocol_send_outgoing_commands(host, event, 1);

        CHECK_RETURN_VALUE(ret)

        //ret = _udp_protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)

        if (UDP_TIME_GREATER_EQUAL(_service_time, timeout))
            return 0;

        do
        {
            _service_time = udp_time_get();

            if (UDP_TIME_GREATER_EQUAL(_service_time, timeout))
                return 0;

            wait_condition =
                static_cast<uint32_t>(SocketWait::RECEIVE) | static_cast<uint32_t>(SocketWait::INTERRUPT);
        } while (wait_condition & static_cast<uint32_t>(SocketWait::INTERRUPT));

        _service_time = udp_time_get();
    } while (wait_condition & static_cast<uint32_t>(SocketWait::RECEIVE));

    return 0;
}

RUdpHost::RUdpHost(const UdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth) :
    _channel_count(channel_count),
    _duplicate_peers(PROTOCOL_MAXIMUM_PEER_ID),
    _incoming_bandwidth(in_bandwidth),
    _maximum_packet_size(HOST_DEFAULT_MAXIMUM_PACKET_SIZE),
    _maximum_waiting_data(HOST_DEFAULT_MAXIMUM_WAITING_DATA),
    _mtu(HOST_DEFAULT_MTU),
    _outgoing_bandwidth(out_bandwidth),
    _received_address(std::make_unique<UdpAddress>()),
    _received_data_length(0),
    _packet_data(2, std::vector<uint8_t>(PROTOCOL_MAXIMUM_MTU))
{
    _conn = std::make_shared<RUdpConnection>(address);

    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID)
    {
        // throw exception
        // ...
    }

    _peer_pod = std::make_unique<RUdpPeerPod>(peer_count);
}

uint32_t
RUdpHost::service_time()
{
    return _service_time;
}
