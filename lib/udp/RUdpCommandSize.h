#ifndef P2P_TECHDEMO_RUDPCOMMANDSIZE_H
#define P2P_TECHDEMO_RUDPCOMMANDSIZE_H

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

#endif // P2P_TECHDEMO_RUDPCOMMANDSIZE_H
