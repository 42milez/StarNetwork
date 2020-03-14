#ifndef P2P_TECHDEMO_LIB_CORE_IO_EVENT_POLL_H_
#define P2P_TECHDEMO_LIB_CORE_IO_EVENT_POLL_H_

#include "lib/core/errors.h"
#include "socket.h"

class EventPoll
{
  private:
    int _fd;

  public:
    Error
    register_event(const SOCKET_PTR &sock);

    Error
    wait_for_receiving(const SOCKET_PTRS &in_sockets, SOCKET_PTRS &out_sockets);

    EventPoll();

    ~EventPoll();
};

#endif // P2P_TECHDEMO_LIB_CORE_IO_EVENT_POLL_H_
