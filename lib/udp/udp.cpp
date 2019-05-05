#include <arpa/inet.h>

#include "core/os.h"
#include "core/singleton.h"

#include "peer.h"
#include "udp.h"

#define IS_PEER_CONNECTED(peer) \
    peer.state != UdpPeerState::CONNECTED && peer.state != UdpPeerState::DISCONNECT_LATER

static uint32_t time_base;

uint32_t udp_time_get()
{
    return Singleton<OS>::Instance().get_ticks_msec() - time_base;
}

void udp_time_set(uint32_t new_time_base)
{
    time_base = Singleton<OS>::Instance().get_ticks_msec() - new_time_base;
}

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
    {
        ip = IpAddress("*");
    }
    else
    {
        ip.set_ipv6(address->host);
    }

    return socket->bind(ip, address->port);
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

UdpCompressor::UdpCompressor() : compress(nullptr), decompress(nullptr), destroy(nullptr)
{}

UdpEvent::UdpEvent() : type(UdpEventType::NONE),
                       channel_id(-1),
                       data(0)
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
    compressor = std::make_shared<UdpCompressor>();

    socket = std::make_unique<Socket>();
    socket->open(Socket::Type::UDP, IP::Type::ANY);
    socket->set_blocking_enabled(false);
}

UdpOutgoingCommand::UdpOutgoingCommand() : reliable_sequence_number(0),
                                           unreliable_sequence_number(0),
                                           sent_time(0),
                                           round_trip_timeout(0),
                                           round_trip_timeout_limit(0),
                                           fragment_offset(0),
                                           fragment_length(0),
                                           send_attempts(0)
{}

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
                     total_waiting_data(0),
                     incoming_peer_id(0),
                     connect_id(0)
{
    data = nullptr;
}
