#ifndef P2P_TECHDEMO_RUDPCHAMBER_H
#define P2P_TECHDEMO_RUDPCHAMBER_H

#include <array>

#include "RUdpBuffer.h"
#include "RUdpCommand.h"
#include "RUdpCommon.h"

class RUdpChamber
{
private:
    using CmdBufIt = std::array<RUdpProtocolTypeSP, PROTOCOL_MAXIMUM_SEGMENT_COMMANDS>::iterator;
    using DataBufIt = std::array<std::shared_ptr<RUdpBuffer>, BUFFER_MAXIMUM>::iterator;

public:
    RUdpChamber();
    const CmdBufIt EmptyCommandBuffer();
    const DataBufIt EmptyDataBuffer();
    bool SendingContinues(RUdpChamber::CmdBufIt cmd_it, RUdpChamber::DataBufIt buf_it, uint32_t mtu,
                          const std::shared_ptr<OutgoingCommand> &outgoing_command);
    int Write(std::vector<uint8_t> &out);

    inline void DropSentTime()
    { buffers_.at(0)->Size((size_t) &((RUdpProtocolHeader *) nullptr)->sent_time); }

    inline void IncrementSegmentSize(size_t val)
    { segment_size_ += val; }

    inline void SetHeader(const VecUInt8SP &header)
    { buffers_.at(0)->Add(header, 0, 4); }

    //bool command_buffer_have_enough_space(RUdpProtocolType *command);
    //bool data_buffer_have_enough_space(RUdpBuffer *buffer);

public:
    inline void buffer_count(size_t val)
    { buffer_count_ = val; }

    inline size_t command_count()
    { return command_count_; }

    inline void command_count(size_t val)
    { command_count_ = val; }

    inline bool continue_sending()
    { return continue_sending_; }

    inline void continue_sending(bool val)
    { continue_sending_ = val; }

    inline uint16_t header_flags()
    { return header_flags_; }

    inline void header_flags(uint16_t val)
    { header_flags_ = val; }

    inline size_t segment_size()
    { return segment_size_; }

    inline void segment_size(size_t val)
    { segment_size_ = val; }

    //void update_buffer_count(const RUdpBuffer *buffer);
    //void update_command_count(const RUdpProtocolType *command);

private:
    std::array<std::shared_ptr<RUdpBuffer>, BUFFER_MAXIMUM> buffers_;
    std::array<RUdpProtocolTypeSP, PROTOCOL_MAXIMUM_SEGMENT_COMMANDS> commands_;

    size_t buffer_count_;
    size_t command_count_;
    size_t segment_size_;

    uint16_t header_flags_;

    bool continue_sending_;
};

#endif // P2P_TECHDEMO_RUDPCHAMBER_H
