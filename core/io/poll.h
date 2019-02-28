#ifndef P2P_TECHDEMO_CORE_IO_EVENTPOLL_H
#define P2P_TECHDEMO_CORE_IO_EVENTPOLL_H

#include "core/base/errors.h"
#include "socket.h"

class EventPoll
{
private:
    int fd;

public:
    Error register_event(int fd, const SOCKET_PTR &socket);

    EventPoll();

    ~EventPoll();
};

#endif // P2P_TECHDEMO_CORE_IO_EVENTPOLL_H
