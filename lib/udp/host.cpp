#include "core/hash.h"

#include "udp.h"

#define IS_PEER_CONNECTED(peer) \
    peer.state != UdpPeerState::CONNECTED && peer.state != UdpPeerState::DISCONNECT_LATER

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

    (*current_peer)->channels = std::move(std::vector<UdpChannel>(static_cast<int>(channel_count)));

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

    std::shared_ptr<UdpProtocol> cmd = std::make_shared<UdpProtocol>();

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

void
UdpHost::_udp_host_bandwidth_throttle()
{
    auto time_current = udp_time_get();
    auto time_elapsed = time_current - _bandwidth_throttle_epoch;
    auto peers_remaining = _connected_peers;
    auto data_total = ~0u;
    auto bandwidth = ~0u;
    auto throttle = 0;
    auto bandwidth_limit = 0;
    auto needs_adjustment = _bandwidth_limited_peers > 0 ? true : false;

    if (time_elapsed < HOST_BANDWIDTH_THROTTLE_INTERVAL)
        return;

    _bandwidth_throttle_epoch = time_current;

    if (peers_remaining == 0)
        return;

    //  Throttle outgoing bandwidth
    // --------------------------------------------------

    if (_outgoing_bandwidth != 0)
    {
        data_total = 0;
        bandwidth = _outgoing_bandwidth * (time_elapsed / 1000);

        for (const auto &peer : _peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            data_total += peer.outgoing_data_total;
        }
    }

    //  Throttle peer bandwidth : Case A ( adjustment is needed )
    // --------------------------------------------------

    while (peers_remaining > 0 && needs_adjustment)
    {
        needs_adjustment = false;

        if (data_total <= bandwidth)
            throttle = PEER_PACKET_THROTTLE_SCALE;
        else
            throttle = (bandwidth * PEER_PACKET_THROTTLE_SCALE) / data_total;

        for (auto &peer : _peers)
        {
            uint32_t peer_bandwidth;

            if ((IS_PEER_CONNECTED(peer)) ||
                peer.incoming_bandwidth == 0 ||
                peer.outgoing_bandwidth_throttle_epoch == time_current)
            {
                continue;
            }

            peer_bandwidth = peer.incoming_bandwidth * (time_elapsed / 1000);
            if ((throttle * peer.outgoing_data_total) / PEER_PACKET_THROTTLE_SCALE <= peer_bandwidth)
                continue;

            peer.packet_throttle_limit = (peer_bandwidth * PEER_PACKET_THROTTLE_SCALE) / peer.outgoing_data_total;

            if (peer.packet_throttle_limit == 0)
                peer.packet_throttle_limit = 1;

            if (peer.packet_throttle > peer.packet_throttle_limit)
                peer.packet_throttle = peer.packet_throttle_limit;

            peer.outgoing_bandwidth_throttle_epoch = time_current;
            peer.incoming_data_total = 0;
            peer.outgoing_data_total = 0;

            needs_adjustment = true;

            --peers_remaining;

            bandwidth -= peer_bandwidth;
            data_total -= peer_bandwidth;
        }
    }

    //  Throttle peer bandwidth : Case B ( adjustment is NOT needed )
    // --------------------------------------------------

    if (peers_remaining > 0)
    {
        if (data_total <= bandwidth)
            throttle = PEER_PACKET_THROTTLE_SCALE;
        else
            throttle = (bandwidth * PEER_PACKET_THROTTLE_SCALE) / data_total;

        for (auto &peer : _peers)
        {
            if ((IS_PEER_CONNECTED(peer)) || peer.outgoing_bandwidth_throttle_epoch == time_current)
                continue;

            peer.packet_throttle_limit = throttle;

            if (peer.packet_throttle > peer.packet_throttle_limit)
                peer.packet_throttle = peer.packet_throttle_limit;

            peer.incoming_data_total = 0;
            peer.outgoing_data_total = 0;
        }
    }

    //  Recalculate Bandwidth Limits
    // --------------------------------------------------

    if (_recalculate_bandwidth_limits)
    {
        _recalculate_bandwidth_limits = false;
        peers_remaining = _connected_peers;
        bandwidth = _incoming_bandwidth;
        needs_adjustment = true;

        if (bandwidth == 0)
            bandwidth_limit = 0;
        else
            while (peers_remaining > 0 && needs_adjustment)
            {
                needs_adjustment = false;
                bandwidth_limit = bandwidth / peers_remaining;

                for (auto &peer: _peers)
                {
                    if ((IS_PEER_CONNECTED(peer)) || peer.incoming_bandwidth_throttle_epoch == time_current)
                        continue;

                    if (peer.outgoing_bandwidth > 0 && peer.outgoing_bandwidth >= bandwidth_limit)
                        continue;

                    peer.incoming_bandwidth_throttle_epoch = time_current;

                    needs_adjustment = true;

                    --peers_remaining;

                    bandwidth -= peer.outgoing_bandwidth;
                }
            }


        std::shared_ptr<UdpProtocol> cmd;

        for (auto &peer : _peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            cmd->header.command = PROTOCOL_COMMAND_BANDWIDTH_LIMIT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
            cmd->header.channel_id = 0xFF;
            cmd->bandwidth_limit.outgoing_bandwidth = htonl(_outgoing_bandwidth);

            if (peer.incoming_bandwidth_throttle_epoch == time_current)
                cmd->bandwidth_limit.incoming_bandwidth = htonl(peer.outgoing_bandwidth);
            else
                cmd->bandwidth_limit.incoming_bandwidth = htonl(bandwidth_limit);

            udp_peer_queue_outgoing_command(peer, cmd, nullptr, 0, 0);
        }
    }
}

int
UdpHost::_udp_protocol_dispatch_incoming_commands(UdpEvent &event)
{
    while (!_dispatch_queue.empty())
    {

    }

    return 0;
}

int
UdpHost::udp_host_service(UdpEvent &event, uint32_t timeout)
{
#define CHECK_RETURN_VALUE(val) \
    if (val == 1) \
        return 1; \
    else if (val == -1) \
        return -1;

    int ret;

    if (event.type == UdpEventType::NONE)
    {
        ret = _udp_protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)
    }

    _service_time = udp_time_get();

    timeout += _service_time;

    if (UDP_TIME_DIFFERENCE(_service_time, _bandwidth_throttle_epoch) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
        _udp_host_bandwidth_throttle();

    //ret = protocol_send_outgoing_commands(host, event, 1);

    CHECK_RETURN_VALUE(ret)

    //ret = protocol_receive_incoming_commands(host, event);

    CHECK_RETURN_VALUE(ret)

    //ret = protocol_send_outgoing_commands(host, event, 1);

    CHECK_RETURN_VALUE(ret)

    ret = _udp_protocol_dispatch_incoming_commands(event);

    CHECK_RETURN_VALUE(ret)

    if (UDP_TIME_GREATER_EQUAL(_service_time, timeout))
        return 0;

    _service_time = udp_time_get();

    return 0;
}

UdpHost::UdpHost(const UdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth) :
    _bandwidth_limited_peers(0),
    _bandwidth_throttle_epoch(0),
    _buffer_count(0),
    _channel_count(channel_count),
    _command_count(0),
    _commands(PROTOCOL_MAXIMUM_PACKET_COMMANDS),
    _connected_peers(0),
    _duplicate_peers(PROTOCOL_MAXIMUM_PEER_ID),
    _incoming_bandwidth(in_bandwidth),
    _maximum_packet_size(HOST_DEFAULT_MAXIMUM_PACKET_SIZE),
    _maximum_waiting_data(HOST_DEFAULT_MAXIMUM_WAITING_DATA),
    _mtu(HOST_DEFAULT_MTU),
    _outgoing_bandwidth(out_bandwidth),
    _peer_count(peer_count),
    _peers(peer_count),
    _recalculate_bandwidth_limits(0),
    _received_address(std::make_unique<UdpAddress>()),
    _received_data_length(0),
    _total_received_data(0),
    _total_received_packets(0),
    _total_sent_data(0),
    _total_sent_packets(0),
    _packet_data(2, std::vector<uint8_t>(PROTOCOL_MAXIMUM_MTU))
{
    _compressor = std::make_shared<UdpCompressor>();

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
        peer.host = this;
        peer.incoming_peer_id = hash32();
        peer.outgoing_session_id = peer.incoming_session_id = 0xFF;
        peer.data = nullptr;

        udp_peer_reset(peer);
    }
}
