#include <errno.h>

#include "socket_util.h"

std::shared_ptr<Socket>
SocketUtil::create_socket(Socket::Type sock_type, IP::Type ip_type)
{
    return std::make_shared<Socket>(sock_type, ip_type);
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