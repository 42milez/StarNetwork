#include <arpa/inet.h>

#include "peer.h"
#include "udp.h"

void
udp_address_set_ip(const std::unique_ptr<UdpAddress> &address, const uint8_t *ip, size_t size)
{
    auto len = size > 16 ? 16 : size;
    memset(address->host, 0, 16);
    memcpy(address->host, ip, len); // network byte-order (big endian)
}

Error
udp_socket_bind(std::unique_ptr<Socket> &socket, const std::unique_ptr<UdpAddress> &address)
{
    IpAddress ip{};

    if (address->wildcard)
        ip = IpAddress("*");
    else
        ip.set_ipv6(address->host);

    return socket->bind(ip, address->port);
}

void
udp_host_compress(std::shared_ptr<UdpHost> &host)
{
    if (host->compressor->destroy != nullptr)
    {
        host->compressor->destroy();
    }
}

void
udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor)
{
    if (compressor)
    {
        host->compressor = compressor;
    }
}

std::shared_ptr<UdpHost>
udp_host_create(const std::unique_ptr<UdpAddress> &address, size_t peer_count, SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    std::shared_ptr<UdpHost> host;

    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID)
        return nullptr;

    host = std::make_shared<UdpHost>(channel_count, in_bandwidth, out_bandwidth, peer_count);

    if (host == nullptr)
        return nullptr;

    if (host->socket == nullptr || (address != nullptr && udp_socket_bind(host->socket, address) != Error::OK))
        return nullptr;

    for (auto &peer : host->peers)
    {
        peer.host = host;

        uuid_generate_time(peer.incoming_peer_id);

        peer.outgoing_session_id = peer.incoming_session_id = 0xFF;
        peer.data = nullptr;

        udp_peer_reset(peer);
    }

    return host;
}

Error
udp_host_connect(std::shared_ptr<UdpHost> &host, const UdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto current_peer = host->peers.begin();
    UdpProtocol command;

    for (; current_peer != host->peers.end(); ++current_peer)
    {
        if (current_peer->state == UdpPeerState::DISCONNECTED)
            break;
    }

    if (current_peer == host->peers.end())
        return Error::CANT_CREATE;

    current_peer->channels = std::move(std::vector<UdpChannel>(static_cast<int>(channel_count)));

    if (current_peer->channels.empty())
        return Error::CANT_CREATE;

    current_peer->state = UdpPeerState::CONNECTING;
    current_peer->address = address;

    uuid_generate_time(current_peer->connect_id);

    if (host->outgoing_bandwidth == 0)
        current_peer->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
        current_peer->window_size = (host->outgoing_bandwidth / UDP_PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (current_peer->window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
        current_peer->window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (current_peer->window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
        current_peer->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;

    command.header.command = PROTOCOL_COMMAND_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    command.header.channel_id = 0xFF;

    uuid_copy(command.connect.outgoing_peer_id, current_peer->incoming_peer_id);

    command.connect.incoming_session_id = current_peer->incoming_session_id;
    command.connect.outgoing_session_id = current_peer->outgoing_session_id;
    command.connect.mtu = htonl(current_peer->mtu);
    command.connect.window_size = htonl(current_peer->window_size);
    command.connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
    command.connect.incoming_bandwidth = htonl(host->incoming_bandwidth);
    command.connect.outgoing_bandwidth = htonl(host->outgoing_bandwidth);
    command.connect.packet_throttle_interval = htonl(current_peer->packet_throttle_interval);
    command.connect.packet_throttle_acceleration = htonl(current_peer->packet_throttle_acceleration);
    command.connect.packet_throttle_deceleration = htonl(current_peer->packet_throttle_deceleration);
    command.connect.data = data;

    auto empty_packet = std::make_shared<UdpPacket>();
    udp_peer_queue_outgoing_command(*current_peer, command, empty_packet, 0, 0);

    return Error::OK;
}

UdpAddress::UdpAddress() : port(0), wildcard(0)
{
    memset(&host, 0, sizeof(host));
}

UdpChannel::UdpChannel() : reliable_windows(PEER_RELIABLE_WINDOWS),
                           outgoing_reliable_sequence_number(0),
                           outgoing_unreliable_seaquence_number(0),
                           incoming_reliable_sequence_number(0),
                           incoming_unreliable_sequence_number(0),
                           used_reliable_windows(0)
{}

UdpHost::UdpHost(SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth, size_t peer_count) :
    bandwidth_limited_peers(0),
    bandwidth_throttle_epoch(0),
    buffer_count(0),
    channel_count(channel_count),
    checksum(nullptr),
    command_count(0),
    connected_peers(0),
    duplicate_peers(PROTOCOL_MAXIMUM_PEER_ID),
    incoming_bandwidth(in_bandwidth),
    intercept(nullptr),
    maximum_packet_size(HOST_DEFAULT_MAXIMUM_PACKET_SIZE),
    maximum_waiting_data(HOST_DEFAULT_MAXIMUM_WAITING_DATA),
    mtu(HOST_DEFAULT_MTU),
    outgoing_bandwidth(out_bandwidth),
    peer_count(peer_count),
    peers(peer_count),
    recalculate_bandwidth_limits(0),
    received_address(std::make_unique<UdpAddress>()),
    received_data(nullptr),
    received_data_length(0),
    total_received_data(0),
    total_received_packets(0),
    total_sent_data(0),
    total_sent_packets(0)
{
    compressor->compress = nullptr;
    compressor->decompress = nullptr;
    compressor->destroy = nullptr;

    socket = std::make_unique<Socket>();
    socket->open(Socket::Type::UDP, IP::Type::ANY);
    socket->set_blocking_enabled(false);
}

UdpPeer::UdpPeer() : outgoing_peer_id(0),
                     outgoing_session_id(0),
                     incoming_session_id(0),
                     state(UdpPeerState::DISCONNECTED),
                     incoming_bandwidth(0),
                     outgoing_bandwidth(0),
                     incoming_bandwidth_throttle_epoch(0),
                     outgoing_bandwidth_throttle_epoch(0),
                     incoming_data_total(0),
                     outgoing_data_total(0),
                     last_send_time(0),
                     last_receive_time(0),
                     next_timeout(0),
                     earliest_timeout(0),
                     packet_loss_epoch(0),
                     packets_sent(0),
                     packets_lost(0),
                     packet_loss(0),
                     packet_loss_variance(0),
                     packet_throttle(0),
                     packet_throttle_limit(0),
                     packet_throttle_counter(0),
                     packet_throttle_epoch(0),
                     packet_throttle_acceleration(0),
                     packet_throttle_deceleration(0),
                     packet_throttle_interval(0),
                     ping_interval(0),
                     timeout_limit(0),
                     timeout_minimum(0),
                     timeout_maximum(0),
                     last_round_trip_time(0),
                     last_round_trip_time_variance(0),
                     lowest_round_trip_time(0),
                     highest_round_trip_time_variance(0),
                     round_trip_time(0),
                     round_trip_time_variance(0),
                     mtu(0),
                     window_size(0),
                     reliable_data_in_transit(0),
                     outgoing_reliable_sequence_number(0),
                     needs_dispatch(0),
                     incoming_unsequenced_group(0),
                     outgoing_unsequenced_group(0),
                     event_data(0),
                     total_waiting_data(0)
{
    uuid_clear(incoming_peer_id);
    uuid_clear(connect_id);
    data = nullptr;
}
