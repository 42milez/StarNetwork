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
        return "NOT A COMMAND";

    auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(data_);

    if (protocol == nullptr)
        return "INVALID COMMAND";

    auto cmd_number = protocol->header.command & PROTOCOL_COMMAND_MASK;

    if (COMMANDS_AS_STRING.size() <= cmd_number)
        return "INVALID COMMAND";

    return COMMANDS_AS_STRING.at(cmd_number);
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
            core::Singleton<core::Logger>::Instance().Debug("[command was copied to the buffer] {0}",
                                                            ProtocolCommandAsString());
        }

        return it + size_;
    }

    auto payload = std::get<static_cast<int>(BufferVariant::VecUInt8SharedPtr)>(data_);

    if (size_ > 0)
    {
        memcpy(&(*it), &(payload->at(0)), size_);
        core::Singleton<core::Logger>::Instance().Debug("[payload was copied to the buffer]");
    }

    return it + size_;
}
