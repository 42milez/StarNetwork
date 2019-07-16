#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <memory>
#include <variant>
#include <vector>

#include <cstddef>

#include "RUdpCommon.h"

class RUdpBuffer
{
public:
    std::vector<uint8_t>::iterator CopyTo(std::vector<uint8_t>::iterator it)
    { return std::copy(data_.begin(), data_.end(), it); }

    inline void Push(const RUdpProtocolTypeSP &cmd)
    {
        data_.emplace_back(cmd); // TODO: Does this call the copy constructor?
    }

    inline void Push(const std::shared_ptr<DataRange> &range)
    { data_.emplace_back(range); }

    inline size_t Size()
    { return data_.size(); }

private:
    std::vector<const std::variant<const RUdpProtocolTypeSP, std::shared_ptr<DataRange>>> data_;
};

using RUdpBufferSP = std::shared_ptr<RUdpBuffer>;

#endif // P2P_TECHDEMO_RUDPBUFFER_H
