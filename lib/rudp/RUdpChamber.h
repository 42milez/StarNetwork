#ifndef P2P_TECHDEMO_RUDPCHAMBER_H
#define P2P_TECHDEMO_RUDPCHAMBER_H

#include <array>

#include "RUdpBuffer.h"
#include "RUdpCommand.h"
#include "RUdpCommon.h"

class RUdpChamber
{
private:
    std::array<std::shared_ptr<RUdpBuffer>, BUFFER_MAXIMUM> _buffers;

    std::array<std::shared_ptr<RUdpProtocolType>, PROTOCOL_MAXIMUM_SEGMENT_COMMANDS> _commands;

    size_t _command_count;

    size_t _buffer_count;

    size_t _segment_size;

    uint16_t _header_flags;

    bool _continue_sending;

public:
    RUdpChamber();

    const std::shared_ptr<RUdpBuffer> EmptyDataBuffer();

    const std::shared_ptr<RUdpProtocolType> EmptyCommandBuffer();

    void segment_size(size_t val);

    size_t segment_size();

    void command_count(size_t val);

    size_t command_count();

    void buffer_count(size_t val);

    void update_segment_size(size_t val);

    bool sending_continues(RUdpProtocolType *command,
                           RUdpBuffer *buffer,
                           uint32_t mtu,
                           const std::shared_ptr<OutgoingCommand> &outgoing_command);

    uint16_t header_flags();

    void header_flags(uint16_t val);

    void update_command_count(const RUdpProtocolType *command);

    void update_buffer_count(const RUdpBuffer *buffer);

    bool continue_sending();

    void continue_sending(bool val);

    bool command_buffer_have_enough_space(RUdpProtocolType *command);

    bool data_buffer_have_enough_space(RUdpBuffer *buffer);

    //void set_data_length(size_t val);

    void Reset();

    int write(std::vector<uint8_t> &out);

    void copy_header_data(const uint8_t *header_data, int header_data_size);
};

#endif // P2P_TECHDEMO_RUDPCHAMBER_H
