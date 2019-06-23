#include "RUdpCommand.h"

OutgoingCommand::OutgoingCommand()
    : fragment_length(),
      fragment_offset(),
      reliable_sequence_number(),
      round_trip_timeout(),
      round_trip_timeout_limit(),
      send_attempts(),
      sent_time(),
      unreliable_sequence_number()
{}
