#ifndef P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H
#define P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H

#include "RUdpCommand.h"

class RUdpIncomingCommand : public RUdpCommand
{
public:
    RUdpIncomingCommand();

    Error
    ResizeFragmentBuffer(size_t val);

public:
    void
    fragment_count(uint32_t val) { fragment_count_ = val; }

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

#endif // P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H
