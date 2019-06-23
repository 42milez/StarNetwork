#include "RUdpChannel.h"
#include "RUdpCommon.h"

RUdpChannel::RUdpChannel()
    : reliable_windows(PEER_RELIABLE_WINDOWS),
      outgoing_reliable_sequence_number(),
      outgoing_unreliable_seaquence_number(),
      incoming_reliable_sequence_number(),
      incoming_unreliable_sequence_number(),
      used_reliable_windows()
{}
