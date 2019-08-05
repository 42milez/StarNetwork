#include "RUdpBuffer.h"

VecUInt8It
RUdpBuffer::CopyTo(VecUInt8It it)
{
    if (data_.index() == static_cast<int>(BufferVariant::RUdpProtocolType))
    {
        auto protocol = std::get_if<static_cast<int>(BufferVariant::RUdpProtocolType)>(&data_);
        memcpy(&*it, &protocol, size_);
    }
    else
    {
        // ...
    }

    return it + size_;
}
