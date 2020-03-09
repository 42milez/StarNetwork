#ifndef P2P_TECHDEMO_RUDPCHAMBER_H
#define P2P_TECHDEMO_RUDPCHAMBER_H

#include <array>

#include "buffer.h"
#include "lib/rudp/command/outgoing_command.h"
#include "macro.h"

namespace rudp
{
    class Chamber
    {
      private:
        using CmdBufIt  = std::array<ProtocolTypeSP, PROTOCOL_MAXIMUM_SEGMENT_COMMANDS>::iterator;
        using DataBufIt = std::array<std::shared_ptr<Buffer>, BUFFER_MAXIMUM>::iterator;

      public:
        Chamber();

        const CmdBufIt
        EmptyCommandBuffer();

        const DataBufIt
        EmptyDataBuffer();

        bool
        SendingContinues(Chamber::CmdBufIt cmd_it, Chamber::DataBufIt buf_it, uint32_t mtu,
                         const std::shared_ptr<OutgoingCommand> &outgoing_command);

        int
        Write(std::vector<uint8_t> &out);

        inline void
        IncrementSegmentSize(size_t val)
        {
            segment_size_ += val;
        }

        inline void
        SetHeader(const std::vector<uint8_t> &header, bool drop_sent_time)
        {
            auto header_size = drop_sent_time ? 2 : 4;
            buffers_.at(0)->CopyHeaderFrom(header, 0, header_size);
        }

        // bool command_buffer_have_enough_space(ProtocolType *command);
        // bool data_buffer_have_enough_space(Buffer *buffer);

      public:
        inline void
        buffer_count(size_t val)
        {
            buffer_count_ = val;
        }

        inline size_t
        command_count()
        {
            return command_count_;
        }

        inline void
        command_count(size_t val)
        {
            command_count_ = val;
        }

        inline bool
        continue_sending()
        {
            return continue_sending_;
        }

        inline void
        continue_sending(bool val)
        {
            continue_sending_ = val;
        }

        inline uint16_t
        header_flags()
        {
            return header_flags_;
        }

        inline void
        header_flags(uint16_t val)
        {
            header_flags_ = val;
        }

        inline size_t
        segment_size()
        {
            return segment_size_;
        }

        inline void
        segment_size(size_t val)
        {
            segment_size_ = val;
        }

        // void update_buffer_count(const Buffer *buffer);
        // void update_command_count(const ProtocolType *command);

      private:
        std::array<std::shared_ptr<Buffer>, BUFFER_MAXIMUM> buffers_;
        std::array<ProtocolTypeSP, PROTOCOL_MAXIMUM_SEGMENT_COMMANDS> commands_;

        size_t buffer_count_;
        size_t command_count_;
        size_t segment_size_;

        uint16_t header_flags_;

        bool continue_sending_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCHAMBER_H
