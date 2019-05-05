#include "command.h"

UdpOutgoingCommand::UdpOutgoingCommand() : reliable_sequence_number(0),
                                           unreliable_sequence_number(0),
                                           sent_time(0),
                                           round_trip_timeout(0),
                                           round_trip_timeout_limit(0),
                                           fragment_offset(0),
                                           fragment_length(0),
                                           send_attempts(0)
{}
