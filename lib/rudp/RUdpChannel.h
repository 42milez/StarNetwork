#ifndef P2P_TECHDEMO_RUDPCHANNEL_H
#define P2P_TECHDEMO_RUDPCHANNEL_H

#include "RUdpCommand.h"

using UdpChannel = struct UdpChannel
{
    uint16_t outgoing_reliable_sequence_number;

    uint16_t outgoing_unreliable_seaquence_number;

    uint16_t used_reliable_windows; // 使用中のバッファ（reliable_windows[PEER_RELIABLE_WINDOWS]）
    std::vector<uint16_t> reliable_windows;

    uint16_t incoming_reliable_sequence_number;

    uint16_t incoming_unreliable_sequence_number;

    std::list<IncomingCommand> incoming_reliable_commands;

    std::list<IncomingCommand> incoming_unreliable_commands;

    UdpChannel();
};

#endif //P2P_TECHDEMO_RUDPCHANNEL_H
