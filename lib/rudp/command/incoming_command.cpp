#include "core/errors.h"
#include "core/logger.h"
#include "core/singleton.h"

#include "incoming_command.h"

namespace rudp
{
    IncomingCommand::IncomingCommand()
            : fragments_(),
              fragment_count_(),
              fragments_remaining_(),
              reliable_sequence_number_(),
              unreliable_sequence_number_()
    {}

    Error
    IncomingCommand::ResizeFragmentBuffer(size_t val) {
        try
        {
            fragments_.resize(val);
        }
        catch (std::bad_alloc &e)
        {
            core::Singleton<core::Logger>::Instance().Critical("BAD ALLOCATION");

            return Error::CANT_ALLOCATE;
        }

        return Error::OK;
    }
} // namespace rudp
