#ifndef P2P_TECHDEMO_UDP_HOST_H
#define P2P_TECHDEMO_UDP_HOST_H

#include "udp.h"

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

#endif // P2P_TECHDEMO_UDP_HOST_H
