#include "RUdpBuffer.h"

void
RUdpBuffer::Push(const std::shared_ptr<RUdpProtocolType> &cmd)
{
    data_.emplace_back(cmd);
}

void
RUdpBuffer::Push(const std::vector<uint8_t> &data)
{
    // TODO: Does this call copy constructor?
    data_.emplace_back(data);
}
