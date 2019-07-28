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
    using VariantBuffer = std::vector<std::variant<RUdpProtocolTypeSP, std::shared_ptr<DataRange>>>;

public:
    VariantBuffer::iterator CopyTo(VariantBuffer::iterator it)
    { return std::copy(data_.begin(), data_.end(), it); }

    inline void Push(const RUdpProtocolTypeSP &cmd)
    {
        data_.emplace_back(cmd); // TODO: Does this call the copy constructor?
    }

    inline void Push(const std::shared_ptr<DataRange> &range)
    { data_.emplace_back(range); }

    size_t Size();

private:
    VariantBuffer data_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
