#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "lib/core/error_macros.h"

#include "socket.h"

namespace core
{
    namespace
    {
        enum class SocketError : int
        {
            ERR_NET_WOULD_BLOCK,
            ERR_NET_IS_CONNECTED,
            ERR_NET_IN_PROGRESS,
            ERR_NET_OTHER
        };

        bool
        can_use_ip(const IP::Type ip_type, const IpAddress &ip_addr, const bool for_bind)
        {
            if (for_bind && !(ip_addr.is_valid() || ip_addr.is_wildcard())) {
                return false;
            }
            else if (!for_bind && !ip_addr.is_valid()) {
                return false;
            }

            // Check if socket support this IP type.
            IP::Type t = ip_addr.is_ipv4() ? IP::Type::V4 : IP::Type::V6;

            if (ip_type != IP::Type::ANY && !ip_addr.is_wildcard() && ip_type != t) {
                return false;
            }

            return true;
        }

        SocketError
        get_socket_error_()
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

        socklen_t
        set_addr_storage(struct sockaddr_storage &addr, const IpAddress &ip, uint16_t port, IP::Type ip_type)
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
        set_ip_port(struct sockaddr_storage &addr, IpAddress &ip, uint16_t &port)
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
    } // namespace

    Socket::Socket()
        : sock_(SOCK_EMPTY)
        , ip_type_(IP::Type::NONE)
        , is_stream_(false)
    {
    }

    Socket::Socket(int sock, IP::Type ip_type, bool is_stream)
        : sock_(sock)
        , ip_type_(ip_type)
        , is_stream_(is_stream)
    {
    }

    std::shared_ptr<Socket>
    Socket::Accept(IpAddress &ip, uint16_t &port)
    {
        std::shared_ptr<Socket> empty;

        ERR_FAIL_COND_V(!IsOpen(), empty);

        struct sockaddr_storage addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t size = sizeof(addr);

        auto fd = ::accept(sock_, (struct sockaddr *)&addr, &size);

        ERR_FAIL_COND_V(fd == SOCK_EMPTY, empty);

        set_ip_port(addr, ip, port);

        std::shared_ptr<Socket> sock = std::make_shared<Socket>(fd, ip_type_, is_stream_);
        sock->SetBlockingEnabled(false);

        return sock;
    }

    Error
    Socket::Bind(const IpAddress &ip, uint16_t port)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED)
        ERR_FAIL_COND_V(!can_use_ip(ip_type_, ip, true), Error::ERR_INVALID_PARAMETER)

        struct sockaddr_storage addr;
        memset(&addr, 0, sizeof(addr));
        auto addr_size = set_addr_storage(addr, ip, port, ip_type_);

        if (::bind(sock_, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY) {
            Close();

            ERR_FAIL_V(Error::ERR_UNAVAILABLE)
        }

        return Error::OK;
    }

    Error
    Socket::Connect(const IpAddress &ip, uint16_t port)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED);
        ERR_FAIL_COND_V(!can_use_ip(ip_type_, ip, false), Error::ERR_INVALID_PARAMETER);

        struct sockaddr_storage addr;
        auto addr_size = set_addr_storage(addr, ip, port, ip_type_);

        if (::connect(sock_, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY) {
            SocketError err = get_socket_error_();

            switch (err) {
                case SocketError::ERR_NET_IS_CONNECTED:
                    return Error::OK;
                case SocketError::ERR_NET_WOULD_BLOCK:
                case SocketError::ERR_NET_IN_PROGRESS:
                    return Error::ERR_BUSY;
                default:
                    ERR_PRINT("Connection to remote host failed")
                    Close();
                    return Error::FAILED;
            }
        }

        return Error::OK;
    }

    Error
    Socket::Listen(int max_pending)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED);

        if (::listen(sock_, max_pending) == SOCK_EMPTY) {
            Close();

            ERR_FAIL_V(Error::FAILED);
        }

        return Error::OK;
    }

    Error
    Socket::Open(SocketType sock_type, IP::Type ip_type)
    {
        ERR_FAIL_COND_V(IsOpen(), Error::ERR_ALREADY_IN_USE);
        ERR_FAIL_COND_V(ip_type > IP::Type::ANY || ip_type < IP::Type::NONE, Error::ERR_INVALID_PARAMETER);

        auto family   = ip_type == IP::Type::V4 ? AF_INET : AF_INET6;
        auto protocol = sock_type == SocketType::TCP ? IPPROTO_TCP : IPPROTO_UDP;
        auto type     = sock_type == SocketType::TCP ? SOCK_STREAM : SOCK_DGRAM;

        sock_ = socket(family, type, protocol);

        if (sock_ == SOCK_EMPTY && ip_type == IP::Type::ANY) {
            ip_type = IP::Type::V4;
            family  = AF_INET;
            sock_   = socket(family, type, protocol);
        }

        ERR_FAIL_COND_V(sock_ == SOCK_EMPTY, Error::FAILED);

        ip_type_ = ip_type;

        if (family == AF_INET6) {
            SetIpv6OnlyEnabled(ip_type != IP::Type::ANY);
        }

        if (protocol == IPPROTO_UDP && ip_type != IP::Type::V6) {
            SetBroadcastingEnabled(true);
        }

        is_stream_ = sock_type == SocketType::TCP;

        return Error::OK;
    }

    // TODO: add implemenration
    Error
    Socket::Poll(PollType type, int timeout)
    {
        // TODO:
        // ...

        return Error::OK;
    }

    Error
    Socket::Recv(uint8_t &buffer, size_t len, ssize_t &bytes_read)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED)

        bytes_read = ::recv(sock_, &buffer, len, 0);

        if (bytes_read < 0) {
            SocketError err = get_socket_error_();

            if (err == SocketError::ERR_NET_WOULD_BLOCK) {
                return Error::ERR_BUSY;
            }

            return Error::FAILED;
        }

        return Error::OK;
    }

    Error
    Socket::RecvFrom(std::vector<uint8_t> &buffer, ssize_t &bytes_read, IpAddress &ip, uint16_t &port)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED);

        struct sockaddr_storage addr;
        socklen_t len = sizeof(struct sockaddr_storage);
        memset(&addr, 0, len);

        bytes_read =
            ::recvfrom(sock_, &(buffer.at(0)), buffer.size(), 0, reinterpret_cast<struct sockaddr *>(&addr), &len);

        if (bytes_read < 0) {
            SocketError err = get_socket_error_();

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
    Socket::Send(const uint8_t &buffer, size_t len, ssize_t &bytes_sent)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED);

        auto flags = 0;

        if (is_stream_) {
            flags = MSG_NOSIGNAL;
        }

        bytes_sent = ::send(sock_, &buffer, len, flags);

        if (bytes_sent < 0) {
            SocketError err = get_socket_error_();

            if (err == SocketError::ERR_NET_WOULD_BLOCK) {
                return Error::ERR_BUSY;
            }

            return Error::FAILED;
        }

        return Error::OK;
    }

    Error
    Socket::SendTo(const void *buffer, size_t len, ssize_t &bytes_sent, const IpAddress &ip, uint16_t port)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED);

        struct sockaddr_storage addr;
        size_t addr_size = set_addr_storage(addr, ip, port, ip_type_);

        bytes_sent = ::sendto(sock_, buffer, len, 0, reinterpret_cast<struct sockaddr *>(&addr), addr_size);

        if (bytes_sent < 0) {
            SocketError err = get_socket_error_();

            if (err == SocketError::ERR_NET_WOULD_BLOCK) {
                return Error::ERR_BUSY;
            }

            return Error::FAILED;
        }

        return Error::OK;
    }

    bool
    Socket::IsOpen() const
    {
        return sock_ != SOCK_EMPTY;
    }

    int
    Socket::AvailableBytes() const
    {
        ERR_FAIL_COND_V(sock_ == SOCK_EMPTY, -1);

        int len;
        auto ret = ::ioctl(sock_, FIONREAD, &len);

        ERR_FAIL_COND_V(ret == -1, 0)

        return len;
    }

    void
    Socket::SetBlockingEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());

        int ret  = 0;
        int opts = ::fcntl(sock_, F_GETFL);

        if (enabled) {
            ret = ::fcntl(sock_, F_SETFL, opts & ~O_NONBLOCK);
        }
        else {
            ret = ::fcntl(sock_, F_SETFL, opts | O_NONBLOCK);
        }

        if (ret != 0) {
            WARN_PRINT("Unable to change non-block mode");
        }
    }

    void
    Socket::SetBroadcastingEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());
        ERR_FAIL_COND(ip_type_ == IP::Type::V6); // IPv6 has no broadcast support.

        int par = enabled ? 1 : 0;

        if (setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, &par, sizeof(par)) != 0) {
            WARN_PRINT("Unable to change broadcast setting");
        }
    }

    void
    Socket::SetIpv6OnlyEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());
        ERR_FAIL_COND(ip_type_ == IP::Type::V4);

        auto par = enabled ? 1 : 0;

        if (setsockopt(sock_, IPPROTO_IPV6, IPV6_V6ONLY, &par, sizeof(int)) != 0) {
            WARN_PRINT("Unable to change IPv4 address mapping over IPv6 option");
        }
    }

    void
    Socket::SetReuseAddressEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());

        auto par = enabled ? 1 : 0;

        if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &par, sizeof(int)) < 0) {
            WARN_PRINT("Unable to socket REUSEADDR option")
        }
    }

    void
    Socket::SetReusePortEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());

        auto par = enabled ? 1 : 0;

        if (setsockopt(sock_, SOL_SOCKET, SO_REUSEPORT, &par, sizeof(int)) < 0) {
            WARN_PRINT("Unable to set socket REUSEPORT option");
        }
    }

    void
    Socket::SetTcpNoDelayEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());
        ERR_FAIL_COND(!is_stream_);

        auto par = enabled ? 1 : 0;

        if (setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, &par, sizeof(int)) < 0) {
            ERR_PRINT("Unable to set TCP no delay option");
        }
    }
} // namespace core
