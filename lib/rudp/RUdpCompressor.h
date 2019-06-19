#ifndef P2P_TECHDEMO_RUDPCOMPRESSOR_H
#define P2P_TECHDEMO_RUDPCOMPRESSOR_H

#include <functional>

#include "RUdpBuffer.h"

using RUdpCompressor = struct RUdpCompressor
{
    std::function<size_t(
        const std::vector<RUdpBuffer> &in_buffers,
        size_t in_limit,
        std::vector<uint8_t> &out_data,
        size_t out_limit)> compress;

    std::function<size_t(
        std::vector<uint8_t> &in_data,
        size_t in_limit,
        std::vector<uint8_t> &out_data,
        size_t out_limit)> decompress;

    std::function<void()> destroy;

    RUdpCompressor();
};

#endif // P2P_TECHDEMO_RUDPCOMPRESSOR_H
