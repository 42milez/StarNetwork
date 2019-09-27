#ifndef P2P_TECHDEMO_RUDPCOMPRESSOR_H
#define P2P_TECHDEMO_RUDPCOMPRESSOR_H

#include <functional>

#include "RUdpBuffer.h"

class RUdpCompressor
{
private:
    std::function<size_t(const std::vector<RUdpBuffer> &in_buffers,
                         size_t in_limit,
                         std::vector<uint8_t> &out_data,
                         size_t out_limit)> compress_;

    std::function<size_t(std::vector<uint8_t> &in_data,
                         size_t in_limit,
                         std::vector<uint8_t> &out_data,
                         size_t out_limit)> decompress_;

    std::function<void()> destroy_;
};

#endif // P2P_TECHDEMO_RUDPCOMPRESSOR_H
