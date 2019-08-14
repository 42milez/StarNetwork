#include "RUdpChannel.h"
#include "RUdpCommon.h"

RUdpChannel::RUdpChannel() : incoming_reliable_sequence_number(),
                             incoming_unreliable_sequence_number(),
                             outgoing_reliable_sequence_number(),
                             outgoing_unreliable_sequence_number(),
                             used_reliable_windows()
{
    std::fill(reliable_windows.begin(), reliable_windows.end(), 0);
}

void RUdpChannel::Reset()
{
    incoming_reliable_sequence_number = 0;
    incoming_unreliable_sequence_number = 0;
    outgoing_reliable_sequence_number = 0;
    outgoing_unreliable_sequence_number = 0;
    used_reliable_windows = 0;

    incoming_reliable_commands.clear();
    incoming_unreliable_commands.clear();

    std::fill(reliable_windows.begin(), reliable_windows.end(), 0);
}
