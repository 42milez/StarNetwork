#ifndef P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H
#define P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H

#include "RUdpCommand.h"

class RUdpIncomingCommand : public RUdpCommand
{
public:
    RUdpIncomingCommand();

private:
    std::vector<uint32_t> fragments_;

    uint32_t fragment_count_;
    uint32_t fragments_remaining_;

    uint16_t reliable_sequence_number_;
    uint16_t unreliable_sequence_number_;
};

#endif // P2P_TECHDEMO_RUDPINCOMINGCOMMAND_H
