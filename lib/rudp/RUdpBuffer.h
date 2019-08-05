#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <cstddef>
#include <memory>
#include <variant>

#include "RUdpCommon.h"

class RUdpBuffer
{
public:
    using VariantBuffer = std::variant<RUdpProtocolTypeSP, VecUInt8SP>;

public:
    inline void Add(const RUdpProtocolTypeSP &data)
    {
        data_ = data;
        size_ = sizeof(RUdpProtocolType);
    }

    inline void Add(const VecUInt8SP &data, size_t offset)
    {
        data_ = data;
        offset_ = offset;
        size_ = data->size() * sizeof(uint8_t);
    }

    VecUInt8It CopyTo(VecUInt8It it);

    size_t Size()
    { return size_; };

private:
    enum class BufferVariant : int
    {
        RUdpProtocolType,
        VecUInt8
    };

private:
    VariantBuffer data_;

    size_t offset_;
    size_t size_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
