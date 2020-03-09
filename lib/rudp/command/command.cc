#include "command.h"

namespace rudp
{
    Command::Command()
        : command_()
        , fragment_length_()
        , fragment_offset_()
        , segment_()
    {
    }

    void
    Command::MoveDataTo(const std::shared_ptr<Buffer> &buffer)
    {
        buffer->CopySegmentFrom(segment_, fragment_offset_, fragment_length_);
    }
} // namespace rudp
