#include <unistd.h>

#include "event_poll.h"

namespace
{
    const int CANNOT_CREATE_EVENT_QUEUE = -1;
    const int CANNOT_REGISTER_EVENT     = -1;
    const int CANNOT_READ_EVENT         = -1;
    const int READ_EVENT_TIMEOUT        = 0;

    const int N_EVENT = 10;
} // namespace

Error
EventPoll::register_event(const SOCKET_PTR &sock)
{
    // TODO:
    // ...

    return Error::OK;
}

Error
EventPoll::wait_for_receiving(const std::vector<SOCKET_PTR> &in_sockets, std::vector<SOCKET_PTR> &out_sockets)
{
    // TODO:
    // ...

    return Error::OK;
}

EventPoll::EventPoll()
{
    // TODO:
    // ...
}

EventPoll::~EventPoll()
{
    close(_fd);
}
