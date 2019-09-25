#ifndef P2P_TECHDEMO_RUDPCOMMANDSIZE_H
#define P2P_TECHDEMO_RUDPCOMMANDSIZE_H

#include <array>

#include "lib/rudp/protocol/RUdpProtocolType.h"

constexpr std::array<size_t, 13> command_sizes{
    0,
    sizeof(RUdpProtocolAcknowledge),
    sizeof(RUdpProtocolConnect),
    sizeof(RUdpProtocolVerifyConnect),
    sizeof(RUdpProtocolDisconnect),
    sizeof(RUdpProtocolPing),
    sizeof(RUdpProtocolSendReliable),
    sizeof(RUdpProtocolSendUnreliable),
    sizeof(RUdpProtocolSendFragment),
    sizeof(RUdpProtocolSendUnsequenced),
    sizeof(RUdpProtocolBandwidthLimit),
    sizeof(RUdpProtocolThrottleConfigure),
    sizeof(RUdpProtocolSendFragment)
};

#endif // P2P_TECHDEMO_RUDPCOMMANDSIZE_H
