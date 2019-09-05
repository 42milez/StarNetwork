#include "RUdpBuffer.h"

VecUInt8It
RUdpBuffer::CopyTo(VecUInt8It it, bool drop_peer_id)
{
    if (data_.index() == static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
    {
        auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(data_);
        memcpy(&(*it), &(*protocol), size_);

        auto debug_aaa = reinterpret_cast<RUdpProtocolType *>(&(*it));

        return it + size_;
    }
    else
    {
        auto data = std::get<static_cast<int>(BufferVariant::VecUInt8SP)>(data_);

        auto debug_data = reinterpret_cast<RUdpProtocolHeader *>(&(data->at(0)));

        if (drop_peer_id)
            memcpy(&(*it), &(data->at(2)), 2);
        else
            memcpy(&(*it), &(data->at(0)), size_);

        return it + size_;
    }
}
