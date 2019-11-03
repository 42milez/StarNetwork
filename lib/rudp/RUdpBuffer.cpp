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
    VecUInt8
};
}

RUdpBuffer::RUdpBuffer()
    : buffer_(),
      offset_(),
      size_()
{}

void
RUdpBuffer::Add(const RUdpProtocolTypeSP &data)
{
    buffer_ = data;
    size_ = COMMAND_SIZES.at(data->header.command & PROTOCOL_COMMAND_MASK);
}

void
RUdpBuffer::CopyHeaderFrom(const VecUInt8 &data, size_t offset, size_t size)
{
    buffer_ = data;
    offset_ = offset;

    if (size != 0)
        size_ = size;
    else
        size_ = data.size() * sizeof(uint8_t);
}

void
RUdpBuffer::CopySegmentFrom(const std::shared_ptr<RUdpSegment> &segment, size_t offset, size_t size)
{
    buffer_ = segment->Data();
    offset_ = offset;

    if (size != 0)
        size_ = size;
    else
        size_ = segment->DataLength();
}

std::string
RUdpBuffer::ProtocolCommandAsString()
{
    if (buffer_.index() != static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
        return "NOT A COMMAND";

    auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(buffer_);

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
    if (buffer_.index() == static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
    {
        auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(buffer_);

        if (protocol)
        {
            memcpy(&(*it), &(*protocol), size_);
            core::Singleton<core::Logger>::Instance().Debug("command was copied to the buffer: {0} ({1})",
                                                            ProtocolCommandAsString(),
                                                            ntohs(protocol->header.reliable_sequence_number));
        }

        return it + size_;
    }

    // TODO: std::vector のコピーが発生する（バッファーに std::variant を使うのはよくない）
    auto payload = std::get<static_cast<int>(BufferVariant::VecUInt8)>(buffer_);

    if (size_ > 0)
    {
        memcpy(&(*it), &(payload.at(0)), size_);
        core::Singleton<core::Logger>::Instance().Debug("payload was copied to the buffer");
    }

    return it + size_;
}
