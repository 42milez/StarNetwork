#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <cstddef>

using UdpBuffer = struct UdpBuffer
{
    void *data;
    size_t data_length;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
