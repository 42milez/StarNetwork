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

void
SocketUnix::_set_socket(SOCKET sock, core::io::IP::Type ip_type, bool is_stream)
{
    _sock = sock;
    _ip_type = ip_type;
    _is_stream = is_stream;
}

bool
SocketUnix::_can_use_ip(const core::io::IpAddress ip_addr, const bool for_bind) const
{
    if (for_bind && !(ip_addr.is_valid() || ip_addr.is_wildcard()))
    {
        return false;
    }
    else if (!for_bind && !ip_addr.is_valid())
    {
        return false;
    }

    // Check if socket support this IP type.
    core::io::IP::Type ip_type = ip_addr.is_ipv4() ? core::io::IP::Type::IPV4 : core::io::IP::Type::IPV6;

    if (_ip_type != core::io::IP::Type::ANY && !ip_addr.is_wildcard() && _ip_type != ip_type)
    {
        return false;
    }

    return true;
}
