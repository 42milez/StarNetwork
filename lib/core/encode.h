#ifndef P2P_TECHDEMO_LIB_CORE_ENCODE_H_
#define P2P_TECHDEMO_LIB_CORE_ENCODE_H_

#include <cstdint>
#include <vector>

namespace core
{
    inline std::vector<uint8_t>
    EncodeUint32(uint32_t val)
    {
        return {
            static_cast<uint8_t>((val >> 0u) & 0xFFu),
            static_cast<uint8_t>((val >> 8u) & 0xFFu),
            static_cast<uint8_t>((val >> 16u) & 0xFFu),
            static_cast<uint8_t>((val >> 24u) & 0xFFu),
        };
    }

    inline uint32_t
    DecodeUint32(std::vector<uint8_t> &val, size_t offset)
    {
        uint32_t ret = 0;

        ret |= val.at(offset);
        ret |= static_cast<uint32_t>(val.at(offset + 1)) << 8u;
        ret |= static_cast<uint32_t>(val.at(offset + 2)) << 16u;
        ret |= static_cast<uint32_t>(val.at(offset + 3)) << 24u;

        return ret;
    }
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_ENCODE_H_
