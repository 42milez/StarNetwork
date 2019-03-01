#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "event_poll.h"

namespace
{
    const int CANNOT_CREATE_EVENT_QUEUE = -1;
    const int CANNOT_REGISTER_EVENT = -1;
    const int CANNOT_READ_EVENT = -1;
    const int READ_EVENT_TIMEOUT = 0;

    const int N_EVENT = 10;

    bool
#ifdef __APPLE__
    is_socket_read(int sock, struct kevent events[], int nfds)
    {
        for (auto i = 0; i < nfds; i++)
        {
            if (events[i].ident == sock)
            {
                return true;
            }
        }

        return false;
    }
#else
    /* Linux */
#endif
}

Error
EventPoll::register_event(const SOCKET_PTR &sock)
{
#ifdef __APPLE__
    struct kevent event{
        static_cast<uintptr_t>(sock->_sock),
        EVFILT_READ,
        EV_ADD | EV_CLEAR,
        0,
        0,
        nullptr
    };

    if (kevent(_fd, &event, 1, nullptr, 0, nullptr) == CANNOT_REGISTER_EVENT)
    {
        // ToDo: logging
        // ...

        return Error::FAILED;
    }
#else
    /* Linux */
#endif

    return Error::OK;
}

Error
EventPoll::wait_for_receiving(const std::vector<SOCKET_PTR> &in_sockets, std::vector<SOCKET_PTR> &out_sockets)
{
#ifdef __APPLE__
    // ToDo: Consider the size of events. The size may affect I/O throughput.
    static struct kevent events[N_EVENT];

    memset(events, 0, sizeof(events));

    auto nfds = kevent(_fd, nullptr, 0, events, N_EVENT, nullptr);

    if (nfds == CANNOT_READ_EVENT)
    {
        return Error::FAILED;
    }
    else if (nfds == READ_EVENT_TIMEOUT)
    {
        // ToDo: handle timeout
        // ...

        return Error::FAILED;
    }
    else
    {
        for (auto i = 0; i < nfds; i++)
        {
            auto s = events[i].ident;

            for (const auto &sock : in_sockets)
            {
                if (s == sock->_sock)
                {
                    out_sockets.push_back(sock);
                }
            }
        }
    }
#else
    /* Linux */
#endif

    return Error::OK;
}

EventPoll::EventPoll()
{
#ifdef __APPLE__
    _fd = kqueue();

    if (_fd == CANNOT_CREATE_EVENT_QUEUE)
    {
        // ToDo: logging
        // ...
    }
#else
    /* Linux */
#endif
}

EventPoll::~EventPoll()
{
    close(_fd);
}
