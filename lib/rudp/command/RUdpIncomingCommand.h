#ifndef P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H
#define P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H

#include "RUdpCommand.h"

namespace rudp
{
    class RUdpIncomingCommand : public RUdpCommand
    {
    public:
        RUdpIncomingCommand();

        Error
        ResizeFragmentBuffer(size_t val);

        inline bool
        IsFragmentAlreadyReceived(uint32_t fragment_number)
        {
            auto idx = fragment_number / 32;
            auto flags = fragments_.at(idx);
            auto flag = 1u << fragment_number % 32;

            return (flags & flag) != 0;
        }

        inline bool
        IsAllFragmentsReceived()
        { return fragments_remaining_ == 0; }

        inline void
        MarkFragmentReceived(uint32_t fragment_number)
        {
            auto idx = fragment_number / 32;
            auto flags = fragments_.at(idx);
            auto flag = 1u << fragment_number % 32;

            --fragments_remaining_;
            fragments_.at(idx) = (flags | flag);
        }

        inline void
        CopyFragmentedPayload(std::vector<uint8_t> &payload)
        { segment_->AppendData(payload); }

    public:
        uint32_t
        fragment_count() { return fragment_count_; }

        void
        fragment_count(uint32_t val) { fragment_count_ = val; }

        uint32_t
        fragments_remaining() { return fragments_remaining_; }

        void
        fragments_remaining(uint32_t val) { fragments_remaining_ = val; }

        uint16_t
        reliable_sequence_number() { return reliable_sequence_number_; }

        void
        reliable_sequence_number(uint16_t val) { reliable_sequence_number_ = val; }

        uint16_t
        unreliable_sequence_number() { return unreliable_sequence_number_; }

        void
        unreliable_sequence_number(uint16_t val) { unreliable_sequence_number_ = val; }

    private:
        std::vector<uint32_t> fragments_;

        uint32_t fragment_count_;
        uint32_t fragments_remaining_;

        uint16_t reliable_sequence_number_;
        uint16_t unreliable_sequence_number_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H
