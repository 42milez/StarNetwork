#include <errno.h>

#include "socket_unix.h"

SocketUnix::NetError
SocketUnix::_get_socket_error()
{
    if (errno == EISCONN)
    {
        return NetError::ERR_NET_IS_CONNECTED;
    }

    if (errno == EINPROGRESS || errno == EALREADY)
    {
        return NetError::ERR_NET_IN_PROGRESS;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
        return NetError::ERR_NET_WOULD_BLOCK;
    }

    // ToDo: logging
    // ...

    return NetError::ERR_NET_OTHER;
}
