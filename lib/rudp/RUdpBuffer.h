#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <cstddef>
#include <memory>
#include <variant>

#include "lib/rudp/protocol/RUdpProtocolType.h"
#include "RUdpType.h"

class RUdpBuffer
{
public:
    RUdpBuffer();

    void
    Add(const RUdpProtocolTypeSP &data);

    void
    Add(const VecUInt8SharedPtr &data, size_t offset, size_t size = 0);

    VecUInt8It
    CopyTo(VecUInt8It it);

    inline size_t
    Size() { return size_; };

    inline void
    Size(size_t val) { size_ = val; }

private:
    enum class BufferVariant : uint8_t
    {
        RUdpProtocolTypeSP,
        VecUInt8SharedPtr
    };

private:
    using VariantBuffer = std::variant<RUdpProtocolTypeSP, VecUInt8SharedPtr>;

    VariantBuffer data_;
    size_t offset_;
    size_t size_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
