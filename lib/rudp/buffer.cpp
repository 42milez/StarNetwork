#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/command/command_size.h"
#include "buffer.h"
#include "const.h"
#include "enum.h"

namespace rudp
{
    namespace
    {
        enum class BufferVariant : uint8_t
        {
            RUdpProtocolTypeSP,
            VecUInt8
        };
    }

    Buffer::Buffer()
            : buffer_(),
              offset_(),
              size_()
    {}

    void
    Buffer::Add(const RUdpProtocolTypeSP &data)
    {
        buffer_ = data;
        size_ = COMMAND_SIZES.at(data->header.command & PROTOCOL_COMMAND_MASK);
    }

    void
    Buffer::CopyHeaderFrom(const VecUInt8 &data, size_t offset, size_t size)
    {
        buffer_ = data;
        offset_ = offset;

        if (size != 0)
            size_ = size;
        else
            size_ = data.size() * sizeof(uint8_t);
    }

    void
    Buffer::CopySegmentFrom(const std::shared_ptr<Segment> &segment, size_t offset, size_t size)
    {
        buffer_ = segment->Data(offset, size);
        offset_ = offset;

        if (size != 0)
            size_ = size;
        else
            size_ = segment->DataLength();
    }

    std::string
    Buffer::ProtocolCommandAsString()
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
    Buffer::CopyTo(VecUInt8It it)
    {
        if (buffer_.index() == static_cast<int>(BufferVariant::RUdpProtocolTypeSP))
        {
            auto protocol = std::get<static_cast<int>(BufferVariant::RUdpProtocolTypeSP)>(buffer_);

            if (protocol)
            {
                memcpy(&(*it), &(*protocol), size_);
                core::Singleton<core::Logger>::Instance().Debug("command was copied to buffer: {0} (sn: {1}, size: {2})",
                        ProtocolCommandAsString(),
                        protocol->header.reliable_sequence_number,
                        size_);
            }

            return it + size_;
        }

        // TODO: std::vector のコピーが発生する（バッファーに std::variant を使うのはよくない）
        auto payload = std::get<static_cast<int>(BufferVariant::VecUInt8)>(buffer_);

        if (size_ > 0)
        {
            memcpy(&(*it), &(payload.at(0)), size_);
            core::Singleton<core::Logger>::Instance().Debug("payload was copied to buffer (size: {0})", size_);
        }

        return it + size_;
    }
} // namespace rudp
