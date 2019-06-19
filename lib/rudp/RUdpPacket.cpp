#include "RUdpPacket.h"
#include "RUdpCommon.h"

uint32_t
RUdpPacket::add_flag(uint32_t flag)
{
    _flags |= flag;

    return _flags;
}

void
RUdpPacket::destroy()
{
    if (_free_callback != nullptr)
        _free_callback(this);

    // TODO: free _data
    // ...
}

size_t
RUdpPacket::data_length()
{
    return _data_length;
}

uint8_t *
RUdpPacket::move_data_pointer(uint32_t val)
{
    return _data += val;
}
