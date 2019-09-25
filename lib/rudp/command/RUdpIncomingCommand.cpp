#include "RUdpIncomingCommand.h"

RUdpIncomingCommand::RUdpIncomingCommand()
    : fragments_(),
      fragment_count_(),
      fragments_remaining_(),
      reliable_sequence_number_(),
      unreliable_sequence_number_()
{}
