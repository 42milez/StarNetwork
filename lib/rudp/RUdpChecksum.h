#ifndef P2P_TECHDEMO_RUDPCHECKSUM_H
#define P2P_TECHDEMO_RUDPCHECKSUM_H

#include "RUdpBuffer.h"

using ChecksumCallback = std::function<uint32_t(const std::vector<UdpBuffer> &buffers, size_t buffer_count)>;

#endif // P2P_TECHDEMO_RUDPCHECKSUM_H
