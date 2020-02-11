#ifndef P2P_TECHDEMO_RUDPCOMMAND_H
#define P2P_TECHDEMO_RUDPCOMMAND_H

#include <vector>

#include "lib/rudp/protocol/RUdpProtocolType.h"
#include "lib/rudp/buffer.h"
#include "lib/rudp/const.h"
#include "lib/rudp/macro.h"
#include "lib/rudp/segment.h"
#include "lib/rudp/type.h"

namespace rudp
{
    class Command
    {
    public:
        Command();

        inline uint8_t
        CommandNumber() { return command_->header.command & PROTOCOL_COMMAND_MASK; }

        inline bool
        HasPayload() { return segment_ != nullptr; }

        inline bool
        IsAcknowledge() { return command_->header.command & static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE); }

        inline bool
        IsUnsequenced() { return command_->header.command & static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_UNSEQUENCED); }

        void
        MoveDataTo(const std::shared_ptr<Buffer> &buffer);

    public:
        inline uint8_t
        header_channel_id() { return command_->header.channel_id; }

        inline void
        header_channel_id(uint8_t val) { command_->header.channel_id = val; }

        inline RUdpProtocolTypeSP
        command() { return command_; }

        inline void
        command(const RUdpProtocolTypeSP &command) { command_ = command; }

        inline uint16_t
        fragment_length() { return fragment_length_; }

        inline void
        fragment_length(uint16_t val) { fragment_length_ = val; }

        inline uint32_t
        fragment_offset() { return fragment_offset_; }

        inline void
        fragment_offset(uint32_t val) { fragment_offset_ = val; }

        inline void
        header_command_number(uint8_t val) { command_->header.command = val; }

        inline void
        header_reliable_sequence_number(uint16_t val) { command_->header.reliable_sequence_number = val; }

        inline SegmentSP &
        segment() { return segment_; }

        inline void
        segment(const SegmentSP &segment) { segment_ = segment; }

        inline void
        send_fragment_data_length(uint16_t val) { command_->send_fragment.data_length = val; }

        inline void
        send_fragment_fragment_count(uint32_t val) { command_->send_fragment.fragment_count = val; }

        inline void
        send_fragment_fragment_number(uint32_t val) { command_->send_fragment.fragment_number = val; }

        inline void
        send_fragment_fragment_offset(uint32_t val) { command_->send_fragment.fragment_offset = val; }

        inline void
        send_fragment_start_sequence_number(uint16_t val) { command_->send_fragment.start_sequence_number = val; }

        inline void
        send_fragment_start_total_length(uint32_t val) { command_->send_fragment.total_length = val; }

        inline void
        send_unreliable_unreliable_sequence_number(uint16_t val) { command_->send_unreliable.unreliable_sequence_number = val; }

        inline void
        send_unsequenced_unsequenced_group(uint16_t val) { command_->send_unsequenced.unsequenced_group = val; }

    protected:
        RUdpProtocolTypeSP command_;
        SegmentSP segment_;

        uint32_t fragment_offset_;
        uint16_t fragment_length_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCOMMAND_H
