#include <algorithm>

#include <cstring>

#include "RUdpChamber.h"
#include "RUdpCommandSize.h"
#include "RUdpProtocol.h"

RUdpChamber::RUdpChamber()
    : _buffer_count(),
      _command_count(),
      _continue_sending(false),
      _header_flags(),
      _segment_size()
{
    for (auto &buf : _buffers)
        buf = std::make_shared<RUdpBuffer>();

    for (auto &cmd : _commands)
        cmd = std::make_shared<RUdpProtocolType>();
}

const std::shared_ptr<RUdpBuffer>
RUdpChamber::EmptyDataBuffer()
{
    if (_buffer_count >= _buffers.size())
        return nullptr;

    return _buffers.at(++_buffer_count);
}

const std::shared_ptr<RUdpProtocolType>
RUdpChamber::EmptyCommandBuffer()
{
    if (_command_count >= _commands.size())
        return nullptr;

    return _commands.at(++_command_count);
}

bool
RUdpChamber::sending_continues(RUdpProtocolType *command,
                               RUdpBuffer *buffer,
                               uint32_t mtu,
                               const std::shared_ptr<OutgoingCommand> &outgoing_command)
{
    // MEMO: [誤] SendReliableOutgoingCommands() では
    //            buffer に protocol_type が挿入されたら同時にインクリメントされるので、
    //            protocol_type か buffer どちらかでよいのでは？
    //       [正] コマンドがパケットを持っている際に buffer がインクリメントされる（コマンドに続くデータがバッファに投入される）ので、
    //            それぞれで判定する必要がある

    // unsent protocol_type exists
    if (command >= &_commands.at(sizeof(_commands) / sizeof(RUdpProtocol)))
        return true;

    // unsent data exists
    if (buffer + 1 >= &_buffers.at(sizeof(_buffers) / sizeof(RUdpBuffer)))
        return true;

    auto command_size = command_sizes[outgoing_command->protocol_type->header.command & PROTOCOL_COMMAND_MASK];

    // has not enough space for protocol_type（コマンド分のスペースがなければ続くデータも送信できないので先にチェック）
    if (mtu - _segment_size < command_size)
        return true;

    if (outgoing_command->segment != nullptr)
        return false;

    // has not enough space for protocol_type with payload
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

//void
//RUdpChamber::set_data_length(size_t val)
//{
//    _buffers[0].Length = val;
//}

int
RUdpChamber::write(std::vector<uint8_t> &out)
{
    auto size = 0;

    for (auto i = 0; i < _buffer_count; ++i) {
        size += _buffers[i].data.size();
    }

    out.resize(size);

    auto it = out.begin();

    for (auto i = 0; i < _buffer_count; ++i) {
        it = std::copy(_buffers[i].data.begin(), _buffers[i].data.end(), it);
    }

    return size;
}

void
RUdpChamber::copy_header_data(const uint8_t *header_data, int header_data_size)
{
    memcpy(_buffers, header_data, header_data_size);
}
