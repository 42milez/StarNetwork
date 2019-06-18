#ifndef P2P_TECHDEMO_RUDPCOMPRESSOR_H
#define P2P_TECHDEMO_RUDPCOMPRESSOR_H

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

#endif // P2P_TECHDEMO_RUDPCOMPRESSOR_H
