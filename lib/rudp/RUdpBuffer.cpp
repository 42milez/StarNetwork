#include "RUdpBuffer.h"

void
RUdpBuffer::Push(const RUdpProtocolTypeSP &cmd)
{
    data_.emplace_back(cmd);
}

void
RUdpBuffer::Push(const std::shared_ptr<ReadRange> &range)
{
    // TODO: Does this call copy constructor?
    data_.emplace_back(range);
}
