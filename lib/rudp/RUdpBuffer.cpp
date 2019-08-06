#include "RUdpBuffer.h"

VecUInt8It
RUdpBuffer::CopyTo(VecUInt8It it)
{
    if (data_.index() == static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
    {
        auto protocol = std::get_if<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(&data_);
        memcpy(&*it, &protocol, size_);
        return it + size_;
    }
    else
    {
        auto data = std::get<static_cast<int>(BufferVariant::VecUInt8SP)>(data_);
        auto current = std::copy(data->begin(), data->end(), it);
        return current;
    }
}
