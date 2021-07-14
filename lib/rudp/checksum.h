#ifndef STAR_NETWORK_LIB_RUDP_CHECKSUM_H_
#define STAR_NETWORK_LIB_RUDP_CHECKSUM_H_

#include "buffer.h"

namespace rudp
{
    using ChecksumCallback = std::function<uint32_t(const std::vector<Buffer> &buffers, size_t buffer_count)>;
} // namespace rudp

#endif // STAR_NETWORK_LIB_RUDP_CHECKSUM_H_
