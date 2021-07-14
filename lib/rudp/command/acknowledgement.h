#ifndef STAR_NETWORK_LIB_RUDP_COMMAND_ACKNOWLEDGEMENT_H_
#define STAR_NETWORK_LIB_RUDP_COMMAND_ACKNOWLEDGEMENT_H_

#include "lib/rudp/protocol/protocol_type.h"

namespace rudp
{
    class Acknowledgement
    {
      public:
        Acknowledgement();

      public:
        inline ProtocolType &
        command()
        {
            return command_;
        }

        inline void
        command(ProtocolType val)
        {
            command_ = val;
        }

        inline uint32_t
        sent_time()
        {
            return sent_time_;
        }

        inline void
        sent_time(uint32_t val)
        {
            sent_time_ = val;
        }

      private:
        ProtocolType command_;
        uint32_t sent_time_;
    };
} // namespace rudp

#endif // STAR_NETWORK_LIB_RUDP_COMMAND_ACKNOWLEDGEMENT_H_
