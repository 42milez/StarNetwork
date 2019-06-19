#include "RUdpChannel.h"
#include "RUdpCommon.h"

RUdpChannel::RUdpChannel() : reliable_windows(PEER_RELIABLE_WINDOWS),
                           outgoing_reliable_sequence_number(0),
                           outgoing_unreliable_seaquence_number(0),
                           incoming_reliable_sequence_number(0),
                           incoming_unreliable_sequence_number(0),
                           used_reliable_windows(0)
{}
