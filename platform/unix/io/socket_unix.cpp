#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include "core/base/error_macros.h"

#include "socket_unix.h"

namespace
{
    int SOCK_EMPTY = -1;
}

bool
SocketUnix::_can_use_ip(const IpAddress &ip_addr, const bool for_bind) const
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
    IP::Type ip_type = ip_addr.is_ipv4() ? IP::Type::V4 : IP::Type::V6;

    if (_ip_type != IP::Type::ANY && !ip_addr.is_wildcard() && _ip_type != ip_type)
    {
        return false;
    }

    return true;
}

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

socklen_t
SocketUnix::_set_addr_storage(struct sockaddr_storage &addr, const IpAddress &ip, uint16_t port, IP::Type ip_type)
{
    memset(&addr, 0, sizeof(struct sockaddr_storage));

    if (ip_type == IP::Type::V6 || ip_type == IP::Type::ANY) // IPv6 socket
    {
        // ToDo: check whether ip is IPv6 only socket with IPv4 address
        // ...

        auto &addr6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = htons(port);

        if (ip.is_valid())
        {
            memcpy(&addr6.sin6_addr.s6_addr, ip.get_ipv6(), 16); // copy 16 bytes
        }
        else
        {
            addr6.sin6_addr = in6addr_any;
        }

        return sizeof(sockaddr_in6);
    }
    else // IPv4 socket
    {
        // ToDo: check if whether ip is IPv4 socket with IPv6 address
        // ...

        auto &addr4 = reinterpret_cast<struct sockaddr_in &>(addr);

        addr4.sin_family = AF_INET;
        addr4.sin_port = htons(port);

        if (ip.is_valid())
        {
            memcpy(&addr4.sin_addr.s_addr, ip.get_ipv4(), 4);
        }
        else
        {
            addr4.sin_addr.s_addr = INADDR_ANY;
        }

        return sizeof(sockaddr_in);
    }
}

void
SocketUnix::_set_ip_port(struct sockaddr_storage &addr, IpAddress &ip, uint16_t &port)
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

void
SocketUnix::set_ipv6_only_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());
    ERR_FAIL_COND(_ip_type == IP::Type::V4);

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, IPPROTO_IPV6, IPV6_V6ONLY, &par, sizeof(int)) != 0)
    {
        WARN_PRINT("Unable to change IPv4 address mapping over IPv6 option");
    }
}

void
SocketUnix::set_reuse_address_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &par, sizeof(int)) < 0)
    {
        WARN_PRINT("Unable to socket REUSEADDR option")
    }
}

void
SocketUnix::set_reuse_port_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEPORT, &par, sizeof(int)) < 0)
    {
        WARN_PRINT("Unable to set socket REUSEPORT option");
    }
}

void
SocketUnix::set_tcp_no_delay_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());
    ERR_FAIL_COND(!_is_stream);

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, &par, sizeof(int)) < 0)
    {
        ERR_PRINT("Unable to set TCP no delay option");
    }
}

void
SocketUnix::_set_socket(SOCKET sock, IP::Type ip_type, bool is_stream)
{
    _sock = sock;
    _ip_type = ip_type;
    _is_stream = is_stream;
}

Error
SocketUnix::bind(const IpAddress &ip, uint16_t port)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);
    ERR_FAIL_COND_V(!_can_use_ip(ip, true), Error::ERR_INVALID_PARAMETER);

    struct sockaddr_storage addr;
    auto addr_size = _set_addr_storage(addr, ip, port, _ip_type);

    if (::bind(_sock, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY)
    {
        close();

        ERR_FAIL_V(Error::ERR_UNAVAILABLE);
    }

    return Error::OK;
}

void
SocketUnix::close()
{
    if (_sock != SOCK_EMPTY)
    {
        ::close(_sock);
    }

    _sock = SOCK_EMPTY;
    _ip_type = IP::Type::NONE;
    _is_stream = false;
}

Error
SocketUnix::connect(const IpAddress &ip, uint16_t port)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);
    ERR_FAIL_COND_V(!_can_use_ip(ip, false), Error::ERR_INVALID_PARAMETER);

    struct sockaddr_storage addr;
    auto addr_size = _set_addr_storage(addr, ip, port, _ip_type);

    if (::connect(_sock, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY)
    {
        NetError err = _get_socket_error();

        switch (err)
        {
            case NetError::ERR_NET_IS_CONNECTED:
                return Error::OK;
            case NetError::ERR_NET_WOULD_BLOCK:
            case NetError::ERR_NET_IN_PROGRESS:
                return Error::ERR_BUSY;
            default:
                ERR_PRINT("Connection to remote host failed")
                close();
                return Error::FAILED;
        }
    }

    return Error::OK;
}

Error
SocketUnix::listen(int max_pending)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);

    if (::listen(_sock, max_pending) == SOCK_EMPTY)
    {
        close();

        ERR_FAIL_V(Error::FAILED);
    }
}

Error
SocketUnix::open(Socket::Type sock_type, IP::Type ip_type)
{
    ERR_FAIL_COND_V(is_open(), Error::ERR_ALREADY_IN_USE);
    ERR_FAIL_COND_V(ip_type > IP::Type::ANY || ip_type < IP::Type::NONE, Error::ERR_INVALID_PARAMETER);

    auto family = ip_type == IP::Type::V4 ? AF_INET : AF_INET6;
    auto protocol = sock_type == Type::TCP ? IPPROTO_TCP : IPPROTO_UDP;
    auto type = sock_type == Type::TCP ? SOCK_STREAM : SOCK_DGRAM;

    _sock = socket(family, type, protocol);

    if (_sock == SOCK_EMPTY && ip_type == IP::Type::ANY)
    {
        ip_type = IP::Type::V4;
        family = AF_INET;
        _sock = socket(family, type, protocol);
    }

    ERR_FAIL_COND_V(_sock == SOCK_EMPTY, Error::FAILED);

    _ip_type = ip_type;

    if (family == AF_INET6)
    {
        set_ipv6_only_enabled(ip_type != IP::Type::ANY);
    }

    if (protocol == IPPROTO_UDP && ip_type != IP::Type::V6)
    {
        set_broadcasting_enabled(true);
    }

    _is_stream = sock_type == Type::TCP;

    return Error::OK;
}

SocketUnix::SocketUnix() : _sock(SOCK_EMPTY), _ip_type(IP::Type::NONE), _is_stream(false) {}

SocketUnix::~SocketUnix()
{
    close();
}
