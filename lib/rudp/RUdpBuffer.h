#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <cstddef>

using RUdpBuffer = struct RUdpBuffer
{
    std::vector<uint8_t> data;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
