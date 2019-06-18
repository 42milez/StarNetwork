#include "RUdpPacket.h"
#include "RUdpCommon.h"

uint32_t
UdpPacket::add_flag(uint32_t flag)
{
    _flags |= flag;

    return _flags;
}

void
UdpPacket::destroy()
{
    if (_free_callback != nullptr)
        _free_callback(this);

    // TODO: free _data
    // ...
}

size_t
UdpPacket::data_length()
{
    return _data_length;
}

uint8_t *
UdpPacket::move_data_pointer(uint32_t val)
{
    return _data += val;
}
