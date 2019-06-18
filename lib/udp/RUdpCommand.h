#ifndef P2P_TECHDEMO_UDP_COMMAND_H
#define P2P_TECHDEMO_UDP_COMMAND_H

#include <vector>

#include "RUdpPacket.h"
#include "RUdpProtocol.h"

using UdpIncomingCommand = struct UdpIncomingCommand
{
    uint16_t reliable_sequence_number;
    uint16_t unreliable_sequence_number;
    std::shared_ptr<UdpProtocolType> command;
    uint32_t fragment_count;
    uint32_t fragments_remaining;
    std::vector<uint32_t> fragments;
    std::shared_ptr<UdpPacket> packet;
};

using UdpOutgoingCommand = struct UdpOutgoingCommand
{
    uint16_t reliable_sequence_number;

    uint16_t unreliable_sequence_number;

    uint32_t sent_time;

    uint32_t round_trip_timeout;

    uint32_t round_trip_timeout_limit;

    uint32_t fragment_offset;

    uint16_t fragment_length;

    uint16_t send_attempts;

    std::shared_ptr<UdpProtocolType> command;

    std::shared_ptr<UdpPacket> packet;

    UdpOutgoingCommand();
};

#endif // P2P_TECHDEMO_UDP_COMMAND_H
