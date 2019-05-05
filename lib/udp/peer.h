#ifndef P2P_TECHDEMO_LIB_UDP_PEER_H
#define P2P_TECHDEMO_LIB_UDP_PEER_H

#include "memory"

#include "udp.h"

UdpOutgoingCommand
udp_peer_queue_outgoing_command(UdpPeer &peer, const std::shared_ptr<UdpProtocol> &command, const std::shared_ptr<UdpPacket> &packet, uint32_t offset, uint16_t length);

void
udp_peer_setup_outgoing_command(UdpPeer &peer, UdpOutgoingCommand &outgoing_command);

void
udp_peer_reset(UdpPeer &peer);

void
udp_peer_reset_queues(UdpPeer &peer);

void
udp_peer_on_disconnect(UdpPeer &peer);

#endif // P2P_TECHDEMO_LIB_UDP_PEER_H
