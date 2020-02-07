#include "RUdpCommand.h"

RUdpCommand::RUdpCommand()
    : command_(),
      fragment_length_(),
      fragment_offset_(),
      segment_()
{}

void
RUdpCommand::MoveDataTo(const std::shared_ptr<RUdpBuffer> &buffer)
{
    buffer->CopySegmentFrom(segment_, fragment_offset_, fragment_length_);
}
