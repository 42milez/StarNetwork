#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/command/RUdpCommandSize.h"
#include "RUdpBuffer.h"
#include "RUdpConst.h"
#include "RUdpEnum.h"

namespace
{
enum class BufferVariant : uint8_t
{
    RUdpProtocolTypeSP,
    VecUInt8SharedPtr
};
}

RUdpBuffer::RUdpBuffer()
    : data_(),
      offset_(),
      size_()
{}

void
RUdpBuffer::Add(const RUdpProtocolTypeSP &data)
{
    data_ = data;
    size_ = COMMAND_SIZES.at(data->header.command & PROTOCOL_COMMAND_MASK);
}

void
RUdpBuffer::Add(const VecUInt8SharedPtr &data, size_t offset, size_t size)
{
    data_ = data;
    offset_ = offset;

    if (size != 0)
        size_ = size;
    else
        size_ = data->size() * sizeof(uint8_t);
}

std::string
RUdpBuffer::ProtocolCommandAsString()
{
    if (data_.index() != static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
        return "not command";

    auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(data_);

    if (protocol == nullptr)
        return "invalid command";

    auto cmd_number = protocol->header.command & PROTOCOL_COMMAND_MASK;

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::NONE))
        return "none";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::ACKNOWLEDGE))
        return "acknowledge";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::CONNECT))
        return "connect";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::VERIFY_CONNECT))
        return "verify_connect";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::DISCONNECT))
        return "disconnect";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::PING))
        return "ping";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::SEND_RELIABLE))
        return "send_reliable";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::SEND_UNRELIABLE))
        return "send_unreliable";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::SEND_FRAGMENT))
        return "send_fragment";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::SEND_UNSEQUENCED))
        return "send_unsequenced";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::BANDWIDTH_LIMIT))
        return "bandwidth_limit";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::THROTTLE_CONFIGURE))
        return "throttle_configure";

    if (cmd_number == static_cast<uint8_t>(RUdpProtocolCommand::SEND_UNRELIABLE_FRAGMENT))
        return "send_unreliable_fragment";

    return "unknown command";
}

VecUInt8It
RUdpBuffer::CopyTo(VecUInt8It it)
{
    if (data_.index() == static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
    {
        auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(data_);

        if (protocol)
        {
            memcpy(&(*it), &(*protocol), size_);

#ifdef DEBUG
            core::Singleton<core::Logger>::Instance().Debug("a command was copied to the buffer: {0}",
                                                            ProtocolCommandAsString());
#endif
        }

        return it + size_;
    }

    auto data = std::get<static_cast<int>(BufferVariant::VecUInt8SharedPtr)>(data_);
    memcpy(&(*it), &(data->at(0)), size_);

    return it + size_;
}
