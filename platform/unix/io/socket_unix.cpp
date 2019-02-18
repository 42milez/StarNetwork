#include <arpa/inet.h>
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

void
SocketUnix::_set_ip_port(struct sockaddr_storage &addr, core::io::IpAddress ip, uint16_t &port)
{
    if (addr.ss_family == AF_INET)
    {
        auto &addr4 = reinterpret_cast<struct sockaddr_in &>(addr);

        auto octet1 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 24);
        auto octet2 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 16);
        auto octet3 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 8);
        auto octet4 = static_cast<uint8_t>(addr4.sin_addr.s_addr);

        ip.set_ipv4({octet1, octet2, octet3, octet4});

        port = ntohs(addr4.sin_port);
    }
    else if (addr.ss_family == AF_INET6)
    {
        auto &addr6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

        ip.set_ipv6(addr6.sin6_addr.s6_addr);

        port = ntohs(addr6.sin6_port);
    }
}
