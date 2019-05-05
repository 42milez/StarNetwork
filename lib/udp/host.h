#ifndef P2P_TECHDEMO_UDP_HOST_H
#define P2P_TECHDEMO_UDP_HOST_H

#include <memory>

#include "core/io/socket.h"
#include "peer.h"
#include "protocol.h"
#include "udp.h"

constexpr int BUFFER_MAXIMUM = 1 + 2 * PROTOCOL_MAXIMUM_PACKET_COMMANDS;

constexpr int HOST_BANDWIDTH_THROTTLE_INTERVAL = 1000;
constexpr int HOST_DEFAULT_MAXIMUM_PACKET_SIZE = 32 * 1024 * 1024;
constexpr int HOST_DEFAULT_MAXIMUM_WAITING_DATA = 32 * 1024 * 1024;
constexpr int HOST_DEFAULT_MTU = 1400;

enum class SysCh : int
{
    CONFIG = 1,
    RELIABLE,
    UNRELIABLE,
    MAX
};

class UdpHost
{
private:
    std::unique_ptr<Socket> _socket;
    uint32_t _incoming_bandwidth;
    uint32_t _outgoing_bandwidth;
    uint32_t _bandwidth_throttle_epoch;
    uint32_t _mtu;
    int _recalculate_bandwidth_limits;
    std::vector<UdpPeer> _peers;
    size_t _peer_count;
    SysCh _channel_count;
    uint32_t _service_time;
    int _continue_sending;
    size_t _packet_size;
    uint16_t _header_flags;
    UdpProtocol _commands[PROTOCOL_MAXIMUM_PACKET_COMMANDS];
    size_t _command_count;
    std::vector<UdpBuffer> _buffers;
    size_t _buffer_count;
    std::shared_ptr<UdpCompressor> _compressor;
    uint8_t _packet_data[2][PROTOCOL_MAXIMUM_MTU];
    std::unique_ptr<UdpAddress> _received_address;
    uint8_t *_received_data;
    size_t _received_data_length;
    uint32_t _total_sent_data;
    uint32_t _total_sent_packets;
    uint32_t _total_received_data;
    uint32_t _total_received_packets;
    size_t _connected_peers;
    size_t _bandwidth_limited_peers;
    size_t _duplicate_peers;
    size_t _maximum_packet_size;
    size_t _maximum_waiting_data;

public:
    UdpHost(SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth, size_t peer_count);

    void
    udp_host_compress(std::shared_ptr<UdpHost> &host);

    void
    udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor);

    std::shared_ptr<UdpHost>
    udp_host_create(const std::unique_ptr<UdpAddress> &address, size_t peer_count, SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth);

    Error
    udp_host_connect(std::shared_ptr<UdpHost> &host, const UdpAddress &address, SysCh channel_count, uint32_t data);

    int
    udp_host_service(std::shared_ptr<UdpHost> &host, UdpEvent &event, uint32_t timeout);

    void
    udp_host_bandwidth_throttle(std::shared_ptr<UdpHost> &host);

    int
    udp_protocol_dispatch_incoming_commands(UdpEvent &event);
};

#endif // P2P_TECHDEMO_UDP_HOST_H
