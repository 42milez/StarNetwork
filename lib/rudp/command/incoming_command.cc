#include "lib/core/errors.h"
#include "lib/core/logger.h"
#include "lib/core/singleton.h"

#include "incoming_command.h"

namespace rudp
{
    IncomingCommand::IncomingCommand()
        : fragments_()
        , fragment_count_()
        , fragments_remaining_()
        , reliable_sequence_number_()
        , unreliable_sequence_number_()
    {
    }

    core::Error
    IncomingCommand::ResizeFragmentBuffer(size_t val)
    {
        try {
            fragments_.resize(val);
        }
        catch (std::bad_alloc &e) {
            core::LOG_CRITICAL("BAD ALLOCATION");

            return core::Error::CANT_ALLOCATE;
        }

        return core::Error::OK;
    }
} // namespace rudp
