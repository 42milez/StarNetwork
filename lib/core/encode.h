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
            static_cast<uint8_t>((val >> 0) & 0xFF),
            static_cast<uint8_t>((val >> 8) & 0xFF),
            static_cast<uint8_t>((val >> 16) & 0xFF),
            static_cast<uint8_t>((val >> 24) & 0xFF),
        };
    }

    inline uint32_t
    DecodeUint32(std::vector<uint8_t> &val, size_t offset)
    {
        uint32_t ret = 0;

        ret |= val.at(offset);
        ret |= static_cast<uint32_t>(val.at(offset + 1)) << 8;
        ret |= static_cast<uint32_t>(val.at(offset + 2)) << 16;
        ret |= static_cast<uint32_t>(val.at(offset + 3)) << 24;

        return ret;
    }
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_ENCODE_H_

