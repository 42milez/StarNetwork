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
    { data_ = cmd; }

    inline void Add(const std::shared_ptr<DataRange> &range)
    { data_ = range; }

    size_t Size();

private:
    enum class BufferVariant : int
    {
        RUdpProtocolType,
        DataRange
    };

private:
    VariantBuffer data_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
