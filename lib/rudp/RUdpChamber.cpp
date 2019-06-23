#include "RUdpChamber.h"
#include "RUdpCommandSize.h"
#include "RUdpProtocol.h"

RUdpChamber::RUdpChamber()
    : _continue_sending(false),
      _header_flags(0),
      _buffer_count(0),
      _command_count(0)
{
    memset(_commands, 0, sizeof(_commands));
}

RUdpBuffer *
RUdpChamber::buffer_insert_pos()
{
    return &_buffers[_buffer_count];
}

RUdpProtocolType *
RUdpChamber::command_insert_pos()
{
    return &_commands[_command_count];
}

bool
RUdpChamber::sending_continues(RUdpProtocolType *command,
                               RUdpBuffer *buffer,
                               uint32_t mtu,
                               const std::shared_ptr<OutgoingCommand> &outgoing_command)
{
    // MEMO: [誤] send_reliable_outgoing_commands() では
    //            buffer に command が挿入されたら同時にインクリメントされるので、
    //            command か buffer どちらかでよいのでは？
    //       [正] コマンドがパケットを持っている際に buffer がインクリメントされる（コマンドに続くデータがバッファに投入される）ので、
    //            それぞれで判定する必要がある

    // unsent command exists
    if (command >= &_commands[sizeof(_commands) / sizeof(RUdpProtocol)])
        return true;

    // unsent data exists
    if (buffer + 1 >= &_buffers[sizeof(_buffers) / sizeof(RUdpBuffer)])
        return true;

    auto command_size = command_sizes[outgoing_command->command->header.command & PROTOCOL_COMMAND_MASK];

    // has not enough space for command（コマンド分のスペースがなければ続くデータも送信できないので先にチェック）
    if (mtu - _segment_size < command_size)
        return true;

    if (outgoing_command->segment != nullptr)
        return false;

    // has not enough space for command with payload
    if (static_cast<uint16_t>(mtu - _segment_size) <
        static_cast<uint16_t>(command_size + outgoing_command->fragment_length)) {
        return true;
    }

    return false;
}

uint16_t
RUdpChamber::header_flags()
{
    return _header_flags;
}

void
RUdpChamber::header_flags(uint16_t val)
{
    _header_flags = val;
}

void
RUdpChamber::increase_segment_size(size_t val)
{
    _segment_size += val;
}

void
RUdpChamber::update_command_count(const RUdpProtocolType *command)
{
    _command_count = command - _commands;
}

void
RUdpChamber::update_buffer_count(const RUdpBuffer *buffer)
{
    _buffer_count = buffer - _buffers;
}

bool
RUdpChamber::continue_sending()
{
    return _continue_sending;
}

void
RUdpChamber::continue_sending(bool val)
{
    _continue_sending = val;
}

bool
RUdpChamber::command_buffer_have_enough_space(RUdpProtocolType *command)
{
    return command < &_commands[PROTOCOL_MAXIMUM_SEGMENT_COMMANDS];
}

bool
RUdpChamber::data_buffer_have_enough_space(RUdpBuffer *buffer)
{
    return buffer < &_buffers[BUFFER_MAXIMUM];
}

void
RUdpChamber::command_count(size_t val)
{
    _command_count = val;
}

size_t
RUdpChamber::command_count()
{
    return _command_count;
}

void
RUdpChamber::buffer_count(size_t val)
{
    _buffer_count = val;
}

void
RUdpChamber::segment_size(size_t val)
{
    _segment_size = val;
}

size_t
RUdpChamber::segment_size()
{
    return _segment_size;
}

void
RUdpChamber::set_data_length(size_t val)
{
    _buffers[0].data_length = val;
}

int
RUdpChamber::write(std::vector<uint8_t> &out)
{
    auto size = 0;

    for (auto i = 0; i < _buffer_count; ++i) {
        size += _buffers[i].data_length;
    }

    out.resize(size);

    int pos = 0;

    for (auto i = 0; i < _buffer_count; ++i) {
        memcpy(&out[pos], _buffers[i].data, _buffers[i].data_length);
        pos += _buffers[i].data_length;
    }

    return size;
}
