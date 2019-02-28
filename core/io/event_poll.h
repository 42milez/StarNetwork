#ifndef P2P_TECHDEMO_CORE_IO_EVENTPOLL_H
#define P2P_TECHDEMO_CORE_IO_EVENTPOLL_H

#include "core/base/errors.h"
#include "socket.h"

class EventPoll
{
private:
    int _fd;

public:
    Error register_event(const SOCKET_PTR &sock);

    Error wait_for_receiving(const SOCKET_PTRS &in_sockets, SOCKET_PTRS &out_sockets);

    EventPoll();

    ~EventPoll();
};

#endif // P2P_TECHDEMO_CORE_IO_EVENTPOLL_H
