#ifndef P2P_TECHDEMO_RUDPCHANNEL_H
#define P2P_TECHDEMO_RUDPCHANNEL_H

#include "RUdpCommand.h"

using RUdpChannel = struct RUdpChannel
{
    RUdpChannel();

    void Reset();

    std::list<IncomingCommand> incoming_reliable_commands;
    std::list<IncomingCommand> incoming_unreliable_commands;

    std::array<uint16_t, PEER_RELIABLE_WINDOWS> reliable_windows;

    uint16_t incoming_reliable_sequence_number;
    uint16_t incoming_unreliable_sequence_number;

    uint16_t outgoing_reliable_sequence_number;
    uint16_t outgoing_unreliable_sequence_number;

    uint16_t used_reliable_windows; // 使用中のバッファ（reliable_windows[PEER_RELIABLE_WINDOWS]）
};

#endif // P2P_TECHDEMO_RUDPCHANNEL_H
