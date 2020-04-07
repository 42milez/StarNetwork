#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "lib/core/error_macros.h"

#include "socket.h"

namespace
{
    enum class SocketError : int
    {
        ERR_NET_WOULD_BLOCK,
        ERR_NET_IS_CONNECTED,
        ERR_NET_IN_PROGRESS,
        ERR_NET_OTHER
    };

#if defined(__APPLE__)
    const int MSG_NOSIGNAL = SO_NOSIGPIPE;
#endif
    const int SOCK_EMPTY = -1;
} // namespace

namespace
{
    SocketError
    _get_socket_error()
    {
        if (errno == EISCONN) {
            return SocketError::ERR_NET_IS_CONNECTED;
        }

        if (errno == EINPROGRESS || errno == EALREADY) {
            return SocketError::ERR_NET_IN_PROGRESS;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return SocketError::ERR_NET_WOULD_BLOCK;
        }

        // TODO: logging
        // ...

        return SocketError::ERR_NET_OTHER;
    }
} // namespace

bool
Socket::_can_use_ip(const IpAddress &ip_addr, const bool for_bind) const
{
    if (for_bind && !(ip_addr.is_valid() || ip_addr.is_wildcard())) {
        return false;
    }
    else if (!for_bind && !ip_addr.is_valid()) {
        return false;
    }

    // Check if socket support this IP type.
    IP::Type ip_type = ip_addr.is_ipv4() ? IP::Type::V4 : IP::Type::V6;

    if (_ip_type != IP::Type::ANY && !ip_addr.is_wildcard() && _ip_type != ip_type) {
        return false;
    }

    return true;
}

socklen_t
Socket::_set_addr_storage(struct sockaddr_storage &addr, const IpAddress &ip, uint16_t port, IP::Type ip_type)
{
    memset(&addr, 0, sizeof(struct sockaddr_storage));

    if (ip_type == IP::Type::V6 || ip_type == IP::Type::ANY) // IPv6 socket
    {
        // TODO: check if ip is IPv6 only socket with IPv4 address
        // ...

        auto &addr6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

        addr6.sin6_family = AF_INET6;
        addr6.sin6_port   = htons(port);

        if (ip.is_valid()) {
            memcpy(&addr6.sin6_addr.s6_addr, ip.GetIPv6(), 16); // copy 16 bytes
        }
        else {
            addr6.sin6_addr = in6addr_any;
        }

        return sizeof(sockaddr_in6);
    }
    else // IPv4 socket
    {
        // TODO: check if ip is IPv4 socket with IPv6 address
        // ...

        auto &addr4 = reinterpret_cast<struct sockaddr_in &>(addr);

        addr4.sin_family = AF_INET;
        addr4.sin_port   = htons(port);

        if (ip.is_valid()) {
            memcpy(&addr4.sin_addr.s_addr, ip.GetIPv4(), 4);
        }
        else {
            addr4.sin_addr.s_addr = INADDR_ANY;
        }

        return sizeof(sockaddr_in);
    }
}

void
Socket::_set_ip_port(struct sockaddr_storage &addr, IpAddress &ip, uint16_t &port)
{
    if (addr.ss_family == AF_INET) {
        auto &addr4 = reinterpret_cast<struct sockaddr_in &>(addr);

        auto octet1 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 24);
        auto octet2 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 16);
        auto octet3 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 8);
        auto octet4 = static_cast<uint8_t>(addr4.sin_addr.s_addr);

        ip.set_ipv4({octet1, octet2, octet3, octet4});

        port = ntohs(addr4.sin_port);
    }
    else if (addr.ss_family == AF_INET6) {
        auto &addr6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

        ip.set_ipv6(addr6.sin6_addr.s6_addr);

        port = ntohs(addr6.sin6_port);
    }
}

void
Socket::_set_socket(SOCKET sock, IP::Type ip_type, bool is_stream)
{
    _sock      = sock;
    _ip_type   = ip_type;
    _is_stream = is_stream;
}

SOCKET_PTR
Socket::accept(IpAddress &ip, uint16_t &port)
{
    SOCKET_PTR empty;

    ERR_FAIL_COND_V(!is_open(), empty);

    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t size = sizeof(addr);

    auto fd = ::accept(_sock, (struct sockaddr *)&addr, &size);

    ERR_FAIL_COND_V(fd == SOCK_EMPTY, empty);

    _set_ip_port(addr, ip, port);

    SOCKET_PTR sock = std::make_shared<Socket>();
    sock->_set_socket(fd, _ip_type, _is_stream);
    sock->set_blocking_enabled(false);

    return sock;
}

Error
Socket::bind(const IpAddress &ip, uint16_t port)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED)
    ERR_FAIL_COND_V(!_can_use_ip(ip, true), Error::ERR_INVALID_PARAMETER)

    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));
    auto addr_size = _set_addr_storage(addr, ip, port, _ip_type);

    if (::bind(_sock, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY) {
        close();

        ERR_FAIL_V(Error::ERR_UNAVAILABLE)
    }

    return Error::OK;
}

void
Socket::close()
{
    if (_sock != SOCK_EMPTY) {
        ::close(_sock);
    }

    _sock      = SOCK_EMPTY;
    _ip_type   = IP::Type::NONE;
    _is_stream = false;
}

Error
Socket::connect(const IpAddress &ip, uint16_t port)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);
    ERR_FAIL_COND_V(!_can_use_ip(ip, false), Error::ERR_INVALID_PARAMETER);

    struct sockaddr_storage addr;
    auto addr_size = _set_addr_storage(addr, ip, port, _ip_type);

    if (::connect(_sock, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY) {
        SocketError err = _get_socket_error();

        switch (err) {
            case SocketError::ERR_NET_IS_CONNECTED:
                return Error::OK;
            case SocketError::ERR_NET_WOULD_BLOCK:
            case SocketError::ERR_NET_IN_PROGRESS:
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
Socket::listen(int max_pending)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);

    if (::listen(_sock, max_pending) == SOCK_EMPTY) {
        close();

        ERR_FAIL_V(Error::FAILED);
    }

    return Error::OK;
}

Error
Socket::open(Socket::Type sock_type, IP::Type ip_type)
{
    ERR_FAIL_COND_V(is_open(), Error::ERR_ALREADY_IN_USE);
    ERR_FAIL_COND_V(ip_type > IP::Type::ANY || ip_type < IP::Type::NONE, Error::ERR_INVALID_PARAMETER);

    auto family   = ip_type == IP::Type::V4 ? AF_INET : AF_INET6;
    auto protocol = sock_type == Type::TCP ? IPPROTO_TCP : IPPROTO_UDP;
    auto type     = sock_type == Type::TCP ? SOCK_STREAM : SOCK_DGRAM;

    _sock = socket(family, type, protocol);

    if (_sock == SOCK_EMPTY && ip_type == IP::Type::ANY) {
        ip_type = IP::Type::V4;
        family  = AF_INET;
        _sock   = socket(family, type, protocol);
    }

    ERR_FAIL_COND_V(_sock == SOCK_EMPTY, Error::FAILED);

    _ip_type = ip_type;

    if (family == AF_INET6) {
        set_ipv6_only_enabled(ip_type != IP::Type::ANY);
    }

    if (protocol == IPPROTO_UDP && ip_type != IP::Type::V6) {
        set_broadcasting_enabled(true);
    }

    _is_stream = sock_type == Type::TCP;

    return Error::OK;
}

// TODO: add implemenration
Error
Socket::poll(Socket::PollType type, int timeout)
{
#ifdef __APPLE__
    // ...
#else /* linux */
    // ...
#endif
    return Error::OK;
}

Error
Socket::recv(uint8_t &buffer, size_t len, ssize_t &bytes_read)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED)

    bytes_read = ::recv(_sock, &buffer, len, 0);

    if (bytes_read < 0) {
        SocketError err = _get_socket_error();

        if (err == SocketError::ERR_NET_WOULD_BLOCK) {
            return Error::ERR_BUSY;
        }

        return Error::FAILED;
    }

    return Error::OK;
}

Error
Socket::recvfrom(std::vector<uint8_t> &buffer, ssize_t &bytes_read, IpAddress &ip, uint16_t &port)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);

    struct sockaddr_storage addr;
    socklen_t len = sizeof(struct sockaddr_storage);
    memset(&addr, 0, len);

    bytes_read = ::recvfrom(_sock, &(buffer.at(0)), buffer.size(), 0, reinterpret_cast<struct sockaddr *>(&addr), &len);

    if (bytes_read < 0) {
        SocketError err = _get_socket_error();

        if (err == SocketError::ERR_NET_WOULD_BLOCK) {
            return Error::ERR_BUSY;
        }

        return Error::FAILED;
    }

    if (addr.ss_family == AF_INET) {
        auto sin = reinterpret_cast<struct sockaddr_in *>(&addr);
        ip.set_ipv4(SPLIT_IPV4_TO_OCTET_INIT_LIST(sin->sin_addr));
        port = ntohs(sin->sin_port);
    }
    else if (addr.ss_family == AF_INET6) {
        auto sin6 = reinterpret_cast<struct sockaddr_in6 *>(&addr);
        ip.set_ipv6(sin6->sin6_addr.s6_addr);
        port = ntohs(sin6->sin6_port);
    }
    else {
        return Error::FAILED;
    }

    return Error::OK;
}

Error
Socket::send(const uint8_t &buffer, size_t len, ssize_t &bytes_sent)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);

    auto flags = 0;

    if (_is_stream) {
        flags = MSG_NOSIGNAL;
    }

    bytes_sent = ::send(_sock, &buffer, len, flags);

    if (bytes_sent < 0) {
        SocketError err = _get_socket_error();

        if (err == SocketError::ERR_NET_WOULD_BLOCK) {
            return Error::ERR_BUSY;
        }

        return Error::FAILED;
    }

    return Error::OK;
}

Error
Socket::sendto(const void *buffer, size_t len, ssize_t &bytes_sent, const IpAddress &ip, uint16_t port)
{
    ERR_FAIL_COND_V(!is_open(), Error::ERR_UNCONFIGURED);

    struct sockaddr_storage addr;
    size_t addr_size = _set_addr_storage(addr, ip, port, _ip_type);

    bytes_sent = ::sendto(_sock, buffer, len, 0, reinterpret_cast<struct sockaddr *>(&addr), addr_size);

    if (bytes_sent < 0) {
        SocketError err = _get_socket_error();

        if (err == SocketError::ERR_NET_WOULD_BLOCK) {
            return Error::ERR_BUSY;
        }

        return Error::FAILED;
    }

    return Error::OK;
}

bool
Socket::is_open() const
{
    return _sock != SOCK_EMPTY;
}

int
Socket::get_available_bytes() const
{
    ERR_FAIL_COND_V(_sock == SOCK_EMPTY, -1);

    int len;
    auto ret = ::ioctl(_sock, FIONREAD, &len);

    ERR_FAIL_COND_V(ret == -1, 0)

    return len;
}

void
Socket::set_blocking_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());

    int ret  = 0;
    int opts = ::fcntl(_sock, F_GETFL);

    if (enabled) {
        ret = ::fcntl(_sock, F_SETFL, opts & ~O_NONBLOCK);
    }
    else {
        ret = ::fcntl(_sock, F_SETFL, opts | O_NONBLOCK);
    }

    if (ret != 0) {
        WARN_PRINT("Unable to change non-block mode");
    }
}

void
Socket::set_broadcasting_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());
    ERR_FAIL_COND(_ip_type == IP::Type::V6); // IPv6 has no broadcast support.

    int par = enabled ? 1 : 0;

    if (setsockopt(_sock, SOL_SOCKET, SO_BROADCAST, &par, sizeof(par)) != 0) {
        WARN_PRINT("Unable to change broadcast setting");
    }
}

void
Socket::set_ipv6_only_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());
    ERR_FAIL_COND(_ip_type == IP::Type::V4);

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, IPPROTO_IPV6, IPV6_V6ONLY, &par, sizeof(int)) != 0) {
        WARN_PRINT("Unable to change IPv4 address mapping over IPv6 option");
    }
}

void
Socket::set_reuse_address_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &par, sizeof(int)) < 0) {
        WARN_PRINT("Unable to socket REUSEADDR option")
    }
}

void
Socket::set_reuse_port_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEPORT, &par, sizeof(int)) < 0) {
        WARN_PRINT("Unable to set socket REUSEPORT option");
    }
}

void
Socket::set_tcp_no_delay_enabled(bool enabled)
{
    ERR_FAIL_COND(!is_open());
    ERR_FAIL_COND(!_is_stream);

    auto par = enabled ? 1 : 0;

    if (setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, &par, sizeof(int)) < 0) {
        ERR_PRINT("Unable to set TCP no delay option");
    }
}

Socket::Socket()
    : _sock(SOCK_EMPTY)
    , _ip_type(IP::Type::NONE)
    , _is_stream(false)
{
}

Socket::~Socket()
{
    close();
}
