#include <errno.h>

#include "socket_util.h"

namespace core
{
    std::shared_ptr<Socket>
    SocketUtil::create_socket()
    {
        return std::make_shared<Socket>();
    }

    bool
    SocketUtil::is_connection_reset_on_recv(ssize_t bytes_read)
    {
        return bytes_read == 0 || (bytes_read == -1 && last_error() == ECONNRESET);
    }

    bool
    SocketUtil::is_no_messages_to_read(ssize_t bytes_read)
    {
        return bytes_read == -1 && SocketUtil::last_error() == EAGAIN;
    }

    int
    SocketUtil::last_error()
    {
        return errno;
    }
} // namespace core
