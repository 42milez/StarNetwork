#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <cstddef>
#include <memory>
#include <variant>

#include "RUdpCommon.h"

class RUdpBuffer
{
public:
    using VariantBuffer = std::variant<RUdpProtocolTypeSP, std::shared_ptr<DataRange>>;

public:
    inline void Add(const RUdpProtocolTypeSP &cmd)
    {
        data_ = cmd;
        size_ = sizeof(RUdpProtocolType);
    }

    inline void Add(const std::shared_ptr<DataRange> &range)
    {
        data_ = range;
        size_ = range->Size();
    }

    void CopyTo(std::vector<uint8_t>::iterator &it);

    size_t Size()
    { return size_; };

private:
    enum class BufferVariant : int
    {
        RUdpProtocolType,
        DataRange
    };

private:
    VariantBuffer data_;

    size_t size_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
