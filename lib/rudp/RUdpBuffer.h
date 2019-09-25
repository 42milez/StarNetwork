#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <cstddef>
#include <memory>
#include <variant>

#include "RUdpCommon.h"

class RUdpBuffer
{
public:
    RUdpBuffer();
    void Add(const RUdpProtocolTypeSP &data);
    void Add(const VecUInt8SP &data, size_t offset, size_t size = 0);
    VecUInt8It CopyTo(VecUInt8It it);

    inline size_t Size()
    { return size_; };

    inline void Size(size_t val)
    { size_ = val; }

private:
    enum class BufferVariant : uint8_t
    {
        RUdpProtocolTypeSP,
        VecUInt8SP
    };

private:
    using VariantBuffer = std::variant<RUdpProtocolTypeSP, VecUInt8SP>;

    VariantBuffer data_;
    size_t offset_;
    size_t size_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
