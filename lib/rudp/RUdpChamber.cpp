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

const RUdpChamber::CmdBufIt
RUdpChamber::EmptyCommandBuffer()
{
    if (_command_count >= _commands.size())
        return nullptr;

    return _commands.begin() + (_command_count++);
}


const RUdpChamber::DataBufIt
RUdpChamber::EmptyDataBuffer()
{
    if (_buffer_count >= _buffers.size())
        return nullptr;

    return _buffers.begin() + (_buffer_count++);
}

bool
RUdpChamber::SendingContinues(const RUdpChamber::CmdBufIt cmd_it,
                              const RUdpChamber::DataBufIt buf_it,
                              uint32_t mtu,
                              const std::shared_ptr<OutgoingCommand> &outgoing_command)
{
    // MEMO: [誤] SendReliableOutgoingCommands() では
    //            buffer に protocol_type が挿入されたら同時にインクリメントされるので、
    //            protocol_type か buffer どちらかでよいのでは？
    //       [正] コマンドがパケットを持っている際に buffer がインクリメントされる（コマンドに続くデータがバッファに投入される）ので、
    //            それぞれで判定する必要がある

    // unsent protocol_type exists
    //if (command >= &_commands.at(sizeof(_commands) / sizeof(RUdpProtocol)))
    if (*cmd_it == nullptr)
        return true;

    // unsent data_ exists
    //if (buffer + 1 >= &_buffers.at(sizeof(_buffers) / sizeof(RUdpBuffer)))
    // If the next is the end of the iterator, no more data cannot be pushed into buffer.
    if (*buf_it == nullptr || (std::next(buf_it, 1) == _buffers.end()))
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
RUdpChamber::update_segment_size(size_t val)
{
    _segment_size += val;
}

//void
//RUdpChamber::update_command_count(const RUdpProtocolType *command)
//{
//    _command_count = command - _commands;
//}

//void
//RUdpChamber::update_buffer_count(const RUdpBuffer *buffer)
//{
//    _buffer_count = buffer - _buffers;
//}

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

//bool
//RUdpChamber::command_buffer_have_enough_space(RUdpProtocolType *command)
//{
//    return command < &_commands[PROTOCOL_MAXIMUM_SEGMENT_COMMANDS];
//}

//bool
//RUdpChamber::data_buffer_have_enough_space(RUdpBuffer *buffer)
//{
//    return buffer < &_buffers[BUFFER_MAXIMUM];
//}

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
RUdpChamber::Write(std::vector<uint8_t> &out)
{
    auto size = 0;

    for (auto &buf : _buffers)
        size += buf->Size();

    out.resize(size);

    auto it = out.begin();

    for (auto &buf : _buffers)
        it = buf->CopyTo(it);

    return size;
}

void
RUdpChamber::SetHeader(const VecUInt8SP &header)
{
    _buffers.at(0)->Add(header, 0);
}
