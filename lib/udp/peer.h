#ifndef P2P_TECHDEMO_LIB_UDP_PEER_H
#define P2P_TECHDEMO_LIB_UDP_PEER_H

#include "udp.h"

constexpr int UDP_PEER_WINDOW_SIZE_SCALE = 64 * 1024;

UdpOutgoingCommand udp_peer_queue_outgoing_command(UdpPeer &peer, const std::shared_ptr<UdpProtocol> &command, const std::shared_ptr<UdpPacket> &packet, uint32_t offset, uint16_t length);

void udp_peer_reset(UdpPeer &peer);

#endif // P2P_TECHDEMO_LIB_UDP_PEER_H
