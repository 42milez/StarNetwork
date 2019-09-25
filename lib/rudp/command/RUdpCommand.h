#ifndef P2P_TECHDEMO_RUDPCOMMAND_H
#define P2P_TECHDEMO_RUDPCOMMAND_H

#include <vector>

#include "lib/rudp/RUdpCommon.h"
#include "lib/rudp/RUdpSegment.h"
#include "lib/rudp/RUdpCommon.h"

class RUdpCommand
{
public:
    inline uint8_t CommandNumber()
    { return command_->header.command & PROTOCOL_COMMAND_MASK; }

    inline bool HasPayload()
    { return segment_ != nullptr; }

    inline bool IsAcknowledge()
    { return command_->header.command & static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE); }

    inline bool IsUnsequenced()
    { return command_->header.command & static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_UNSEQUENCED); }

public:
    inline uint8_t channel_id()
    { return command_->header.channel_id; }

    inline void channel_id(uint8_t val)
    { command_->header.channel_id = val; }

    inline RUdpProtocolTypeSP command()
    { return command_; }

    inline void command(const RUdpProtocolTypeSP &command)
    { command_ = command; }

    inline void command_number(uint8_t val)
    { command_->header.command = val; }

    inline void header_reliable_sequence_number(uint16_t val)
    { command_->header.reliable_sequence_number = val; }

    inline RUdpSegmentSP & segment()
    { return segment_; }

    inline void segment(const RUdpSegmentSP &segment)
    { segment_ = segment; }

    inline void send_fragment_data_length(uint16_t val)
    { command_->send_fragment.data_length = val; }

    inline void send_fragment_fragment_count(uint32_t val)
    { command_->send_fragment.fragment_count = val; }

    inline void send_fragment_fragment_number(uint32_t val)
    { command_->send_fragment.fragment_number = val; }

    inline void send_fragment_fragment_offset(uint32_t val)
    { command_->send_fragment.fragment_offset = val; }

    inline void send_fragment_start_sequence_number(uint16_t val)
    { command_->send_fragment.start_sequence_number = val; }

    inline void send_fragment_start_total_length(uint32_t val)
    { command_->send_fragment.total_length = val; }

    inline void send_unreliable_unreliable_sequence_number(uint16_t val)
    { command_->send_unreliable.unreliable_sequence_number = val; }

    inline void send_unsequenced_unsequenced_group(uint16_t val)
    { command_->send_unsequenced.unsequenced_group = val; }

protected:
    RUdpProtocolTypeSP command_;
    RUdpSegmentSP segment_;
};

#endif // P2P_TECHDEMO_RUDPCOMMAND_H
