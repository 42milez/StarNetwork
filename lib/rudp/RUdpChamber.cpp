#include <algorithm>

#include <cstring>

#include "RUdpChamber.h"
#include "RUdpCommandSize.h"
#include "RUdpProtocol.h"

RUdpChamber::RUdpChamber()
    : buffer_count_(),
      command_count_(),
      continue_sending_(false),
      header_flags_(),
      segment_size_()
{
    for (auto &buf : buffers_)
        buf = std::make_shared<RUdpBuffer>();

    for (auto &cmd : commands_)
        cmd = std::make_shared<RUdpProtocolType>();
}

const RUdpChamber::CmdBufIt
RUdpChamber::EmptyCommandBuffer()
{
    if (command_count_ >= commands_.size())
        return nullptr;

    return commands_.begin() + (command_count_++);
}

const RUdpChamber::DataBufIt
RUdpChamber::EmptyDataBuffer()
{
    if (buffer_count_ >= buffers_.size())
        return nullptr;

    return buffers_.begin() + (buffer_count_++);
}

bool
RUdpChamber::SendingContinues(const RUdpChamber::CmdBufIt cmd_it,
                              const RUdpChamber::DataBufIt buf_it,
                              uint32_t mtu,
                              const std::shared_ptr<OutgoingCommand> &outgoing_command)
{
    // MEMO: [誤] SendReliableOutgoingCommands() では
    //            buffer に command が挿入されたら同時にインクリメントされるので、
    //            command か buffer どちらかでよいのでは？
    //       [正] コマンドがパケットを持っている際に buffer がインクリメントされる（コマンドに続くデータがバッファに投入される）ので、
    //            それぞれで判定する必要がある

    // unsent command exists
    //if (command >= &commands_.at(sizeof(commands_) / sizeof(RUdpProtocol)))
    if (*cmd_it == nullptr)
        return true;

    // unsent data_ exists
    //if (buffer + 1 >= &buffers_.at(sizeof(buffers_) / sizeof(RUdpBuffer)))
    // If the next is the end of the iterator, no more data cannot be pushed into buffer.
    if (*buf_it == nullptr || (std::next(buf_it, 1) == buffers_.end()))
        return true;

    auto command_size = command_sizes[outgoing_command->command->header.command & PROTOCOL_COMMAND_MASK];

    // has not enough space for command（コマンド分のスペースがなければ続くデータも送信できないので先にチェック）
    if (mtu - segment_size_ < command_size)
        return true;

    if (outgoing_command->segment != nullptr)
        return false;

    // has not enough space for command with payload
    if (static_cast<uint16_t>(mtu - segment_size_) <
        static_cast<uint16_t>(command_size + outgoing_command->fragment_length)) {
        return true;
    }

    return false;
}

uint16_t
RUdpChamber::header_flags()
{
    return header_flags_;
}

void
RUdpChamber::header_flags(uint16_t val)
{
    header_flags_ = val;
}

void
RUdpChamber::update_segment_size(size_t val)
{
    segment_size_ += val;
}

bool
RUdpChamber::continue_sending()
{
    return continue_sending_;
}

void
RUdpChamber::continue_sending(bool val)
{
    continue_sending_ = val;
}

void
RUdpChamber::command_count(size_t val)
{
    command_count_ = val;
}

size_t
RUdpChamber::command_count()
{
    return command_count_;
}

void
RUdpChamber::buffer_count(size_t val)
{
    buffer_count_ = val;
}

void
RUdpChamber::segment_size(size_t val)
{
    segment_size_ = val;
}

size_t
RUdpChamber::segment_size()
{
    return segment_size_;
}

int
RUdpChamber::Write(std::vector<uint8_t> &out)
{
    auto size = 0;

    for (auto &buf : buffers_)
        size += buf->Size();

    out.resize(size);

    auto it = out.begin();

    for (auto &buf : buffers_)
        it = buf->CopyTo(it);

    return size;
}

void
RUdpChamber::SetHeader(const VecUInt8SP &header)
{
    buffers_.at(0)->Add(header, 0, 4);
}
