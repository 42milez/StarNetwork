#include "RUdpBuffer.h"

RUdpBuffer::RUdpBuffer()
    : data_(),
      offset_(),
      size_()
{}

void
RUdpBuffer::Add(const RUdpProtocolTypeSP &data)
{
    data_ = data;
    size_ = sizeof(RUdpProtocolType);
}

void
RUdpBuffer::Add(const VecUInt8SharedPtr &data, size_t offset, size_t size)
{
    data_ = data;
    offset_ = offset;

    if (size != 0)
        size_ = size;
    else
        size_ = data->size() * sizeof(uint8_t);
}

VecUInt8It
RUdpBuffer::CopyTo(VecUInt8It it)
{
    if (data_.index() == static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
    {
        auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(data_);
        memcpy(&(*it), &(*protocol), size_);

        return it + size_;
    }

    auto data = std::get<static_cast<int>(BufferVariant::VecUInt8SharedPtr)>(data_);
    memcpy(&(*it), &(data->at(0)), size_);

    return it + size_;
}
