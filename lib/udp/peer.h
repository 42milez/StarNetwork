#ifndef P2P_TECHDEMO_LIB_UDP_PEER_H
#define P2P_TECHDEMO_LIB_UDP_PEER_H

#include "udp.h"

constexpr int UDP_PEER_WINDOW_SIZE_SCALE = 64 * 1024;

UdpOutgoingCommand udp_peer_queue_outgoing_command(UdpPeer &peer, std::unique_ptr<UdpProtocol> &&command, UdpPacket &&packet, uint32_t offset, uint16_t length);

void udp_peer_reset(UdpPeer &peer);

#endif // P2P_TECHDEMO_LIB_UDP_PEER_H
