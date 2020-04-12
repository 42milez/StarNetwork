#include <algorithm>

#include "chamber.h"
#include "lib/core/logger.h"
#include "lib/core/singleton.h"
#include "lib/rudp/command/command_size.h"

namespace rudp
{
    Chamber::Chamber()
        : buffer_count_()
        , command_count_()
        , continue_sending_()
        , header_flags_()
        , segment_size_()
    {
        for (auto &buf : buffers_)
            buf = std::make_shared<Buffer>();

        for (auto &cmd : commands_)
            cmd = std::make_shared<ProtocolType>();
    }

    const Chamber::CmdBufIt
    Chamber::EmptyCommandBuffer()
    {
        if (command_count_ >= commands_.size())
            return nullptr;

        return commands_.begin() + (command_count_++);
    }

    const Chamber::DataBufIt
    Chamber::EmptyDataBuffer()
    {
        if (buffer_count_ >= buffers_.size())
            return nullptr;

        return buffers_.begin() + (buffer_count_++);
    }

    bool
    Chamber::SendingContinues(const Chamber::CmdBufIt cmd_it, const Chamber::DataBufIt buf_it, uint32_t mtu,
                              const std::shared_ptr<OutgoingCommand> &outgoing_command)
    {
        // MEMO: [誤] SendReliableOutgoingCommands() では
        //            buffer に command が挿入されたら同時にインクリメントされるので、
        //            command か buffer どちらかでよいのでは？
        //       [正] コマンドがパケットを持っている際に buffer
        //       がインクリメントされる（コマンドに続くデータがバッファに投入される）ので、
        //            それぞれで判定する必要がある

        // unsent command exists
        // if (command >= &commands_.at(sizeof(commands_) / sizeof(RUdpProtocol)))
        if (cmd_it == nullptr)
            return true;

        // unsent buffer_ exists
        // if (buffer + 1 >= &buffers_.at(sizeof(buffers_) / sizeof(Buffer)))
        // If the next is the end of the iterator, no more data cannot be pushed into buffer.
        if (*buf_it == nullptr || (std::next(buf_it, 1) == buffers_.end()))
            return true;

        auto command_size = COMMAND_SIZES[outgoing_command->CommandNumber()];

        // has not enough space for command（コマンド分のスペースがなければ続くデータも送信できないので先にチェック）
        if (mtu - segment_size_ < command_size)
            return true;

        if (outgoing_command->HasPayload())
            return false;

        // has not enough space for command with payload
        auto has_not_enough_space = static_cast<uint16_t>(mtu - segment_size_) <
                                    static_cast<uint16_t>(command_size + outgoing_command->fragment_length());
        if (has_not_enough_space) {
            return true;
        }

        return false;
    }

    int
    Chamber::Write(std::vector<uint8_t> &out)
    {
        LOG_DEBUG_VA("buffer count: {0}", buffer_count_)

        auto size = 0;
        for (size_t i = 0; i < buffer_count_; ++i)
            size += buffers_.at(i)->Size();

        out.resize(size);

        auto it = out.begin();
        for (size_t i = 0; i < buffer_count_; ++i)
            it = buffers_.at(i)->CopyTo(it);

        return size;
    }
} // namespace rudp
