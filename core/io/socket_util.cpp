#include <errno.h>

#include "socket_util.h"

bool
SocketUtil::is_connection_reset_on_recv(ssize_t read_byte_count)
{
    return read_byte_count == 0 || (read_byte_count == -1 && last_error() == ECONNRESET);
}

int
SocketUtil::last_error()
{
    return errno;
}
