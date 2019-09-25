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
    void Reset();
    bool SendingContinues(RUdpChamber::CmdBufIt cmd_it, RUdpChamber::DataBufIt buf_it, uint32_t mtu,
                          const std::shared_ptr<OutgoingCommand> &outgoing_command);
    void SetHeader(const VecUInt8SP &header);
    int Write(std::vector<uint8_t> &out);

    inline void DropSentTime()
    { buffers_.at(0)->Size((size_t) &((RUdpProtocolHeader *) nullptr)->sent_time); }

    //bool command_buffer_have_enough_space(RUdpProtocolType *command);
    //bool data_buffer_have_enough_space(RUdpBuffer *buffer);

public:
    void buffer_count(size_t val);

    size_t command_count();
    void command_count(size_t val);

    bool continue_sending();
    void continue_sending(bool val);

    uint16_t header_flags();
    void header_flags(uint16_t val);

    size_t segment_size();
    void segment_size(size_t val);

    void update_segment_size(size_t val);

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
