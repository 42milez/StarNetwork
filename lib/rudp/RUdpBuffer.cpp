#include "RUdpBuffer.h"

size_t
RUdpBuffer::Size()
{
    if (data_.index() == static_cast<int>(BufferVariant::RUdpProtocolType))
    {
        return sizeof(RUdpProtocolType);
    }
    else
    {
        return std::get<static_cast<int>(BufferVariant::DataRange)>(data_)->Size();
    }
}
