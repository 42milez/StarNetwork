#ifndef P2P_TECHDEMO_RUDPCOMMAND_H
#define P2P_TECHDEMO_RUDPCOMMAND_H

#include <vector>

#include "RUdpCommon.h"
#include "RUdpSegment.h"

using IncomingCommand = struct IncomingCommand
{
    uint16_t reliable_sequence_number;
    uint16_t unreliable_sequence_number;
    std::shared_ptr<RUdpProtocolType> command;
    uint32_t fragment_count;
    uint32_t fragments_remaining;
    std::vector<uint32_t> fragments;
    std::shared_ptr<RUdpSegment> segment;
};

using OutgoingCommand = struct OutgoingCommand
{
    OutgoingCommand();

    std::shared_ptr<RUdpProtocolType> protocol_type;
    std::shared_ptr<RUdpSegment> segment;

    uint32_t fragment_offset;
    uint32_t round_trip_timeout;
    uint32_t round_trip_timeout_limit;
    uint32_t sent_time;

    uint16_t fragment_length;
    uint16_t reliable_sequence_number;
    uint16_t send_attempts;
    uint16_t unreliable_sequence_number;
};

#endif // P2P_TECHDEMO_RUDPCOMMAND_H
