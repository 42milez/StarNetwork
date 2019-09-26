#include "RUdpChannel.h"

RUdpChannel::RUdpChannel()
    : incoming_reliable_commands_(),
      incoming_unreliable_commands_(),
      reliable_windows_(),
      incoming_reliable_sequence_number_(),
      incoming_unreliable_sequence_number_(),
      outgoing_reliable_sequence_number_(),
      outgoing_unreliable_sequence_number_(),
      used_reliable_windows_()
{}

void RUdpChannel::Reset()
{
    incoming_reliable_commands_.clear();
    incoming_unreliable_commands_.clear();
    std::fill(reliable_windows_.begin(), reliable_windows_.end(), 0);
    incoming_reliable_sequence_number_ = 0;
    incoming_unreliable_sequence_number_ = 0;
    outgoing_reliable_sequence_number_ = 0;
    outgoing_unreliable_sequence_number_ = 0;
    used_reliable_windows_ = 0;
}
