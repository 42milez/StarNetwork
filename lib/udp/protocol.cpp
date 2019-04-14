#include "protocol.h"

constexpr std::array<size_t, 13> command_sizes{
    0,
    sizeof(UdpProtocolAcknowledge),
    sizeof(UdpProtocolConnect),
    sizeof(UdpProtocolVerifyConnect),
    sizeof(UdpProtocolDisconnect),
    sizeof(UdpProtocolPing),
    sizeof(UdpProtocolSendReliable),
    sizeof(UdpProtocolSendUnreliable),
    sizeof(UdpProtocolSendFragment),
    sizeof(UdpProtocolSendUnsequenced),
    sizeof(UdpProtocolBandwidthLimit),
    sizeof(UdpProtocolThrottleConfigure),
    sizeof(UdpProtocolSendFragment)
};

size_t
udp_protocol_command_size(uint8_t command_number)
{
    return command_sizes[command_number & PROTOCOL_COMMAND_MASK];
}
