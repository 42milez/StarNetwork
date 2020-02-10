#ifndef P2P_TECHDEMO_RUDPCHECKSUM_H
#define P2P_TECHDEMO_RUDPCHECKSUM_H

#include "RUdpBuffer.h"

namespace rudp
{
    using ChecksumCallback = std::function<uint32_t(const std::vector<RUdpBuffer> &buffers, size_t buffer_count)>;
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCHECKSUM_H
