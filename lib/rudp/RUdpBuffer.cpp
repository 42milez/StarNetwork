#include "RUdpBuffer.h"

void
RUdpBuffer::Push(const std::shared_ptr<RUdpProtocolType> &cmd)
{
    data_.emplace_back(cmd);
}

void
RUdpBuffer::Push(const VecUInt8SP &data)
{
    // TODO: Does this call copy constructor?
    data_.emplace_back(data);
}
