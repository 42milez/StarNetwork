#ifndef P2P_TECHDEMO_RUDPCHAMBER_H
#define P2P_TECHDEMO_RUDPCHAMBER_H

#include <array>

#include "RUdpBuffer.h"
#include "RUdpCommand.h"
#include "RUdpCommon.h"

class RUdpChamber
{
public:
    RUdpChamber();

    const RUdpProtocolTypeSP EmptyCommandBuffer();
    const RUdpBufferSP EmptyDataBuffer();

    bool sending_continues(const RUdpProtocolTypeSP &command,
                           const std::shared_ptr<RUdpBuffer> &buffer,
                           uint32_t mtu,
                           const std::shared_ptr<OutgoingCommand> &outgoing_command);

    //bool command_buffer_have_enough_space(RUdpProtocolType *command);

    //bool data_buffer_have_enough_space(RUdpBuffer *buffer);

    //void set_data_length(size_t val);

    void Reset();

    int Write(std::vector<uint8_t> &out);

    void SetHeader(const VecUInt8SP &header);

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

    //void update_buffer_count(const RUdpBuffer *buffer);

    //void update_command_count(const RUdpProtocolType *command);

    void update_segment_size(size_t val);

private:
    std::array<RUdpBufferSP, BUFFER_MAXIMUM> _buffers;
    std::array<RUdpProtocolTypeSP, PROTOCOL_MAXIMUM_SEGMENT_COMMANDS> _commands;

    size_t _buffer_count;
    size_t _command_count;
    size_t _segment_size;

    uint16_t _header_flags;

    bool _continue_sending;
};

#endif // P2P_TECHDEMO_RUDPCHAMBER_H
