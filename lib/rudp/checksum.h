#ifndef P2P_TECHDEMO_RUDPCHECKSUM_H
#define P2P_TECHDEMO_RUDPCHECKSUM_H

#include "buffer.h"

namespace rudp
{
    using ChecksumCallback = std::function<uint32_t(const std::vector<Buffer> &buffers, size_t buffer_count)>;
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCHECKSUM_H
