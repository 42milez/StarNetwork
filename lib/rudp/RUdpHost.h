#ifndef P2P_TECHDEMO_RUDPHOST_H
#define P2P_TECHDEMO_RUDPHOST_H

#include <vector>

#include "core/io/socket.h"

#include "RUdpAddress.h"
#include "RUdpPeerPod.h"

class UdpHost
{
private:
    std::vector<uint8_t> _received_data;

    std::vector<std::vector<uint8_t>> _packet_data;

    std::unique_ptr<UdpAddress> _received_address;

    std::unique_ptr<Socket> _socket;

    size_t _received_data_length;

    size_t _duplicate_peers;

    size_t _maximum_packet_size;

    size_t _maximum_waiting_data;

    SysCh _channel_count;

    uint32_t _incoming_bandwidth;

    uint32_t _outgoing_bandwidth;

    uint32_t _mtu;

    uint32_t _service_time;

    std::unique_ptr<UdpPeerPod> _peer_pod;

public:
    UdpHost(const UdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth);

    Error
    udp_host_connect(const UdpAddress &address, SysCh channel_count, uint32_t data);

    int
    udp_host_service(std::unique_ptr<UdpEvent> &event, uint32_t timeout);

    uint32_t service_time();

    Error _udp_socket_bind(std::unique_ptr<Socket> &socket, const UdpAddress &address);

    ssize_t _udp_socket_send(const UdpAddress &address);

    std::shared_ptr<UdpPeer> _pop_peer_from_dispatch_queue();
};

#endif // P2P_TECHDEMO_RUDPHOST_H
