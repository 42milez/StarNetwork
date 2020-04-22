#ifndef P2P_TECHDEMO_LIB_CORE_IO_EVENT_POLL_H_
#define P2P_TECHDEMO_LIB_CORE_IO_EVENT_POLL_H_

#include "lib/core/errors.h"
#include "socket.h"

namespace core
{
    class EventPoll
    {
      private:
        int _fd;

      public:
        Error
        register_event(const std::shared_ptr<Socket> &sock);

        Error
        wait_for_receiving(const std::vector<std::shared_ptr<Socket>> &in_sockets,
                           std::vector<std::shared_ptr<Socket>> &out_sockets);

        EventPoll();

        ~EventPoll();
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_IO_EVENT_POLL_H_
