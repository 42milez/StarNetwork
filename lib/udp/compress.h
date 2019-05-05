#ifndef P2P_TECHDEMO_LIB_UDP_COMPRESS_H
#define P2P_TECHDEMO_LIB_UDP_COMPRESS_H

#include <functional>

#include "udp.h"

using UdpCompressor = struct UdpCompressor
{
    std::function<size_t(
        const std::vector<UdpBuffer> &in_buffers,
        size_t in_limit,
        std::vector<uint8_t> &out_data,
        size_t out_limit)> compress;

    std::function<size_t(
        std::vector<uint8_t> &in_data,
        size_t in_limit,
        std::vector<uint8_t> &out_data,
        size_t out_limit)> decompress;

    std::function<void()> destroy;

    UdpCompressor();
};

int
udp_host_compress_with_range_coder(std::shared_ptr<UdpHost> &host);

#endif // P2P_TECHDEMO_LIB_UDP_COMPRESS_H
