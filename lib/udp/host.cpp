#include "core/error_macros.h"
#include "core/hash.h"

#include "udp.h"

namespace
{
    std::vector<size_t> command_sizes{
        0,
        sizeof(UdpProtocolAcknowledge),
        sizeof(UdpProtocolConnect),
        sizeof(UdpProtocolVerifyConnect),
        sizeof(UdpProtocolDisconnect),
        sizeof(UdpProtocolPing),
        sizeof(UdpProtocolSendReliable),
        sizeof(UdpProtocolSendUnreliable),
        sizeof(UdpProtocolSendFragment),
        sizeof(UdpProtocolSendUnsequenced),
        sizeof(UdpProtocolBandwidthLimit),
        sizeof(UdpProtocolThrottleConfigure),
        sizeof(UdpProtocolSendFragment)
    };
}

void
UdpHost::udp_host_compress(std::shared_ptr<UdpHost> &host)
{
    if (_compressor->destroy != nullptr)
        _compressor->destroy();
}

void
UdpHost::udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor)
{
    if (compressor)
        _compressor = compressor;
}

/*
Error
UdpHost::udp_host_connect(const UdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto current_peer = _peers.begin();

    for (; current_peer != _peers.end(); ++current_peer)
    {
        if ((*current_peer)->state == UdpPeerState::DISCONNECTED)
            break;
    }

    if (current_peer == _peers.end())
        return Error::CANT_CREATE;

    (*current_peer)->channels = std::move(std::vector<std::shared_ptr<UdpChannel>>(static_cast<int>(channel_count)));

    if ((*current_peer)->channels.empty())
        return Error::CANT_CREATE;

    (*current_peer)->state = UdpPeerState::CONNECTING;
    (*current_peer)->address = address;
    (*current_peer)->connect_id = hash32();

    if (_outgoing_bandwidth == 0)
        (*current_peer)->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
        (*current_peer)->window_size = (_outgoing_bandwidth / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;

    if ((*current_peer)->window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
        (*current_peer)->window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

    if ((*current_peer)->window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
        (*current_peer)->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;

    std::shared_ptr<UdpProtocolType> cmd = std::make_shared<UdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    cmd->connect.outgoing_peer_id = htons((*current_peer)->incoming_peer_id);
    cmd->connect.incoming_session_id = (*current_peer)->incoming_session_id;
    cmd->connect.outgoing_session_id = (*current_peer)->outgoing_session_id;
    cmd->connect.mtu = htonl((*current_peer)->mtu);
    cmd->connect.window_size = htonl((*current_peer)->window_size);
    cmd->connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
    cmd->connect.incoming_bandwidth = htonl(_incoming_bandwidth);
    cmd->connect.outgoing_bandwidth = htonl(_outgoing_bandwidth);
    cmd->connect.packet_throttle_interval = htonl((*current_peer)->packet_throttle_interval);
    cmd->connect.packet_throttle_acceleration = htonl((*current_peer)->packet_throttle_acceleration);
    cmd->connect.packet_throttle_deceleration = htonl((*current_peer)->packet_throttle_deceleration);
    cmd->connect.data = data;

    udp_peer_queue_outgoing_command(*current_peer, cmd, nullptr, 0, 0);

    return Error::OK;
}
*/

Error
UdpHost::udp_host_connect(const UdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto peer = _peer_pod->available_peer_exists();

    if (peer == nullptr)
        return Error::CANT_CREATE;

    auto err = peer->setup(address, channel_count, data, _incoming_bandwidth, _outgoing_bandwidth);

    return err;
}

int
UdpHost::udp_host_service(std::unique_ptr<UdpEvent> &event, uint32_t timeout)
{
#define CHECK_RETURN_VALUE(val) \
    if (val == 1)               \
        return 1;               \
    else if (val == -1)         \
        return -1;

    int ret;

    if (event != nullptr)
    {
        event->type = UdpEventType::NONE;
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
        ret = _udp_protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)
    }

    _service_time = udp_time_get();

    timeout += _service_time;

    uint32_t wait_condition;

    do
    {
        // 帯域幅の調整
        if (UDP_TIME_DIFFERENCE(_service_time, _bandwidth_throttle_epoch) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
            _udp_host_bandwidth_throttle();

        //
        ret = _protocol_send_outgoing_commands(event, true);

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
                static_cast<uint32_t>(UdpSocketWait::RECEIVE) | static_cast<uint32_t>(UdpSocketWait::INTERRUPT);
        } while (wait_condition & static_cast<uint32_t>(UdpSocketWait::INTERRUPT));

        _service_time = udp_time_get();
    } while (wait_condition & static_cast<uint32_t>(UdpSocketWait::RECEIVE));

    return 0;
}

UdpHost::UdpHost(const UdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
    :
    _checksum(nullptr),
    _buffer_count(0),
    _channel_count(channel_count),
    _command_count(0),
    _duplicate_peers(PROTOCOL_MAXIMUM_PEER_ID),
    _incoming_bandwidth(in_bandwidth),
    _maximum_packet_size(HOST_DEFAULT_MAXIMUM_PACKET_SIZE),
    _maximum_waiting_data(HOST_DEFAULT_MAXIMUM_WAITING_DATA),
    _mtu(HOST_DEFAULT_MTU),
    _outgoing_bandwidth(out_bandwidth),
    _peer_pod(peer_count),
    _received_address(std::make_unique<UdpAddress>()),
    _received_data_length(0),
    _total_received_data(0),
    _total_received_packets(0),
    _total_sent_data(0),
    _total_sent_packets(0),
    _packet_data(2, std::vector<uint8_t>(PROTOCOL_MAXIMUM_MTU))
{
    memset(_commands, 0, sizeof(_commands));

    _socket = std::make_unique<Socket>();
    _socket->open(Socket::Type::UDP, IP::Type::ANY);
    _socket->set_blocking_enabled(false);

    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID)
    {
        // throw exception
        // ...
    }

    if (_socket == nullptr || (_udp_socket_bind(_socket, address) != Error::OK))
    {
        // throw exception
        // ...
    }

    for (auto &peer : _peers)
    {
        peer->host = this;
        peer->incoming_peer_id = hash32();
        peer->outgoing_session_id = peer->incoming_session_id = 0xFF;
        peer->data = nullptr;

        udp_peer_reset(peer);
    }
}

uint32_t
UdpHost::service_time()
{
    return _service_time;
}

Error
UdpHost::_udp_socket_bind(std::unique_ptr<Socket> &socket, const UdpAddress &address)
{
    IpAddress ip{};

    if (address.wildcard)
        ip = IpAddress("*");
    else
        ip.set_ipv6(address.host);

    return socket->bind(ip, address.port);
}

ssize_t
UdpHost::_udp_socket_send(const UdpAddress &address)
{
    IpAddress dest;

    dest.set_ipv6(address.host);

    std::vector<uint8_t> out;

    int size = 0;

    for (auto i = 0; i < _buffer_count; ++i)
    {
        size += _buffers[i].data_length;
    }

    out.resize(size);

    int pos = 0;

    for (auto i = 0; i < _buffer_count; ++i)
    {
        memcpy(&out[pos], _buffers[i].data, _buffers[i].data_length);
        pos += _buffers[i].data_length;
    }

    ssize_t sent = 0;

    auto err = _socket->sendto(out, size, sent, dest, address.port);

    if (err != Error::OK)
    {
        if (err == Error::ERR_BUSY)
            return 0;

        return -1;
    }

    return sent;
}
