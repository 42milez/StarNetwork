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

    inline void Add(const VecUInt8SP &data, size_t offset, size_t size = 0)
    {
        data_ = data;
        offset_ = offset;

        if (size != 0)
            size_ = size;
        else
            size_ = data->size() * sizeof(uint8_t);
    }

    VecUInt8It CopyTo(VecUInt8It it);

    [[nodiscard]]
    size_t Size() const
    { return size_; };

private:
    enum class BufferVariant : int
    {
        RUdpProtocolTypeSP,
        VecUInt8SP
    };

private:
    VariantBuffer data_;

    size_t offset_;
    size_t size_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
