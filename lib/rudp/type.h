#ifndef P2P_TECHDEMO_RUDPTYPE_H
#define P2P_TECHDEMO_RUDPTYPE_H

#include <vector>

namespace rudp
{
    using VecUInt8 = std::vector<uint8_t>;
    using VecUInt8It = std::vector<uint8_t>::iterator;
    using VecUInt8RawPtr = std::vector<uint8_t> *;
    using VecUInt8SharedPtr = std::shared_ptr<std::vector<uint8_t>>;
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPTYPE_H