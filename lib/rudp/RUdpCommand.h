#ifndef P2P_TECHDEMO_RUDPCOMMAND_H
#define P2P_TECHDEMO_RUDPCOMMAND_H

#include <vector>

#include "RUdpCommon.h"
#include "RUdpSegment.h"

using IncomingCommand = struct IncomingCommand
{
    RUdpProtocolTypeSP command;
    std::vector<uint32_t> fragments;
    std::shared_ptr<RUdpSegment> segment;

    uint32_t fragment_count;
    uint32_t fragments_remaining;

    uint16_t reliable_sequence_number;
    uint16_t unreliable_sequence_number;
};

using OutgoingCommand = struct OutgoingCommand
{
    OutgoingCommand();

    RUdpProtocolTypeSP command;
    std::shared_ptr<RUdpSegment> segment;

    uint16_t fragment_length;
    uint32_t fragment_offset;

    uint16_t reliable_sequence_number;
    uint16_t unreliable_sequence_number;

    uint32_t round_trip_timeout;
    uint32_t round_trip_timeout_limit;

    uint16_t send_attempts;
    uint32_t sent_time;
};

#endif // P2P_TECHDEMO_RUDPCOMMAND_H
