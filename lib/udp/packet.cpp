#include "packet.h"
#include "udp.h"

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
