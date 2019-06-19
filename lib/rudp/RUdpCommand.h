#ifndef P2P_TECHDEMO_RUDPCOMMAND_H
#define P2P_TECHDEMO_RUDPCOMMAND_H

#include <vector>

#include "RUdpCommon.h"
#include "RUdpPacket.h"

using IncomingCommand = struct IncomingCommand
{
    uint16_t reliable_sequence_number;
    uint16_t unreliable_sequence_number;
    std::shared_ptr<RUdpProtocolType> command;
    uint32_t fragment_count;
    uint32_t fragments_remaining;
    std::vector<uint32_t> fragments;
    std::shared_ptr<RUdpPacket> packet;
};

using OutgoingCommand = struct OutgoingCommand
{
    uint16_t reliable_sequence_number;

    uint16_t unreliable_sequence_number;

    uint32_t sent_time;

    uint32_t round_trip_timeout;

    uint32_t round_trip_timeout_limit;

    uint32_t fragment_offset;

    uint16_t fragment_length;

    uint16_t send_attempts;

    std::shared_ptr<RUdpProtocolType> command;

    std::shared_ptr<RUdpPacket> packet;

    OutgoingCommand();
};

#endif // P2P_TECHDEMO_RUDPCOMMAND_H
