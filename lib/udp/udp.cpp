#include <uuid/uuid.h>

#include "peer.h"
#include "udp.h"

void
udp_address_set_ip(const std::unique_ptr<UdpAddress> &address, const uint8_t *ip, size_t size)
{
    auto len = size > 16 ? 16 : size;
    memset(address->host, 0, 16);
    memcpy(address->host, ip, len); // network byte-order (big endian)
}

int
udp_socket_bind(std::unique_ptr<Socket> &socket, const std::unique_ptr<UdpAddress> &address)
{
    IpAddress ip;

    if (address->wildcard)
    {
        ip = IpAddress("*");
    }
    else
    {
        ip.set_ipv6(address->host);
    }

    if (socket->bind(ip, address->port) != Error::OK)
    {
        return -1;
    }

    return 0;
}

void
udp_host_compress(std::shared_ptr<UdpHost> &host)
{
    if (host->compressor->context != nullptr && host->compressor->destroy != nullptr)
    {
        host->compressor->destroy(host->compressor->context);
    }
}

void
udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor)
{
    if (compressor)
    {
        host->compressor = compressor;
    }
    else
    {
        host->compressor->context = nullptr;
    }
}

std::shared_ptr<UdpHost>
udp_host_create(std::unique_ptr<UdpAddress> &&address, size_t peer_count, SysCh channel_limit, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    std::shared_ptr<UdpHost> host;

    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID)
    {
        return nullptr;
    }

    host = std::make_shared<UdpHost>(address, channel_limit, in_bandwidth, out_bandwidth, peer_count);

    if (host == nullptr)
    {
        return nullptr;
    }

    if (host->socket == nullptr || (address != nullptr && udp_socket_bind(host->socket, address) < 0))
    {
        return nullptr;
    }

    //udp_socket_set_option(host->socket, SOCKOPT_NONBLOCK, 1);
    //udp_socket_set_option(host->socket, SOCKOPT_BROADCAST, 1);
    //udp_socket_set_option(host->socket, SOCKOPT_RCVBUF, HOST_RECEIVE_BUFFER_SIZE);
    //udp_socket_set_option(host->socket, SOCKOPT_SNDBUF, HOST_SEND_BUFFER_SIZE);

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

UdpAddress::UdpAddress() : port(0), wildcard(0)
{
    memset(&host, 0, sizeof(host));
}

UdpHost::UdpHost(std::unique_ptr<UdpAddress> &&address, SysCh channel_limit, uint32_t in_bandwidth, uint32_t out_bandwidth, size_t peer_count) :
    address(std::move(address)),
    bandwidth_limited_peers(0),
    bandwidth_throttle_epoch(0),
    buffer_count(0),
    channel_limit(channel_limit),
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
    compressor->context = nullptr;
    compressor->compress = nullptr;
    compressor->decompress = nullptr;
    compressor->destroy = nullptr;

    socket = std::make_unique<Socket>();
    socket->open(Socket::Type::UDP, IP::Type::ANY);
    socket->set_blocking_enabled(false);
}
