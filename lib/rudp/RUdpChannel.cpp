#include "RUdpChannel.h"
#include "RUdpCommon.h"

RUdpChannel::RUdpChannel()
    : incoming_reliable_sequence_number(),
      incoming_unreliable_sequence_number(),
      outgoing_reliable_sequence_number(),
      outgoing_unreliable_sequence_number(),
      reliable_windows(PEER_RELIABLE_WINDOWS),
      used_reliable_windows()
{}
