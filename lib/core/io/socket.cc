#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
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
        const size_t EPOLL_MAX_EVENTS   = 64;
        const int EPOLL_ERROR_ON_CREATE = -1;
        const int EPOLL_ERROR_ON_CTL    = -1;
        const int EPOLL_ERROR_ON_WAIT   = -1;
        const int EPOLL_BLOCKING        = -1;
        const int EPOLL_NON_BLOCKING    = 0;
        const int EPOLL_TIMEOUT         = 10; // msec

        enum class SocketError : int
        {
            ERR_NET_WOULD_BLOCK,
            ERR_NET_IS_CONNECTED,
            ERR_NET_IN_PROGRESS,
            ERR_NET_OTHER
        };

        bool
        CanUseIp(const IP::Type ip_type, const IpAddress &ip_addr, const bool for_bind)
        {
            if (for_bind && !(ip_addr.IsValid() || ip_addr.IsWildcard())) {
                return false;
            }
            else if (!for_bind && !ip_addr.IsValid()) {
                return false;
            }

            // Check if socket support this IP type.
            IP::Type t = ip_addr.IsValid() ? IP::Type::V4 : IP::Type::V6;

            if (ip_type != IP::Type::ANY && !ip_addr.IsWildcard() && ip_type != t) {
                return false;
            }

            return true;
        }

        SocketError
        GetSocketError()
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
        SetAddrStorage(struct sockaddr_storage &addr, const IpAddress &ip, uint16_t port, IP::Type ip_type)
        {
            memset(&addr, 0, sizeof(struct sockaddr_storage));

            if (ip_type == IP::Type::V6 || ip_type == IP::Type::ANY) // IPv6 socket
            {
                // TODO: check if ip is IPv6 only socket with IPv4 address
                // ...

                auto &addr6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

                addr6.sin6_family = AF_INET6;
                addr6.sin6_port   = htons(port);

                if (ip.IsValid()) {
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

                if (ip.IsValid()) {
                    memcpy(&addr4.sin_addr.s_addr, ip.GetIPv4(), 4);
                }
                else {
                    addr4.sin_addr.s_addr = INADDR_ANY;
                }

                return sizeof(sockaddr_in);
            }
        }

        void
        SetIpPort(struct sockaddr_storage &addr, IpAddress &ip, uint16_t &port)
        {
            if (addr.ss_family == AF_INET) {
                auto &addr4 = reinterpret_cast<struct sockaddr_in &>(addr);

                auto octet1 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 24);
                auto octet2 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 16);
                auto octet3 = static_cast<uint8_t>(addr4.sin_addr.s_addr >> 8);
                auto octet4 = static_cast<uint8_t>(addr4.sin_addr.s_addr);

                ip.SetIpv4({octet1, octet2, octet3, octet4});

                port = ntohs(addr4.sin_port);
            }
            else if (addr.ss_family == AF_INET6) {
                auto &addr6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

                ip.SetIpv6(addr6.sin6_addr.s6_addr);

                port = ntohs(addr6.sin6_port);
            }
        }
    } // namespace

    Socket::Socket()
        : ip_type_(IP::Type::NONE)
        , is_stream_(false)
        , sfd_(SOCK_EMPTY)
    {
    }

    Socket::Socket(int sock, IP::Type ip_type, bool is_stream)
        : ip_type_(ip_type)
        , is_stream_(is_stream)
        , sfd_(sock)
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

        auto fd = ::accept(sfd_, (struct sockaddr *)&addr, &size);

        ERR_FAIL_COND_V(fd == SOCK_EMPTY, empty);

        SetIpPort(addr, ip, port);

        std::shared_ptr<Socket> sock = std::make_shared<Socket>(fd, ip_type_, is_stream_);
        sock->SetBlockingEnabled(false);

        return sock;
    }

    Error
    Socket::Bind(const IpAddress &ip, uint16_t port)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED)
        ERR_FAIL_COND_V(!CanUseIp(ip_type_, ip, true), Error::ERR_INVALID_PARAMETER)

        struct sockaddr_storage addr;
        memset(&addr, 0, sizeof(addr));
        auto addr_size = SetAddrStorage(addr, ip, port, ip_type_);

        if (::bind(sfd_, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY) {
            Close();

            ERR_FAIL_V(Error::ERR_UNAVAILABLE)
        }

        return Error::OK;
    }

    void
    Socket::Close()
    {
        ::epoll_ctl(efd_, EPOLL_CTL_DEL, sfd_, NULL);

        if (sfd_ != SOCK_EMPTY) {
            ::close(sfd_);
        }

        if (efd_ != SOCK_EMPTY) {
            ::close(efd_);
        }

        efd_       = SOCK_EMPTY;
        sfd_       = SOCK_EMPTY;
        ip_type_   = IP::Type::NONE;
        is_stream_ = false;
    }

    Error
    Socket::Connect(const IpAddress &ip, uint16_t port)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED);
        ERR_FAIL_COND_V(!CanUseIp(ip_type_, ip, false), Error::ERR_INVALID_PARAMETER);

        struct sockaddr_storage addr;
        auto addr_size = SetAddrStorage(addr, ip, port, ip_type_);

        if (::connect(sfd_, reinterpret_cast<struct sockaddr *>(&addr), addr_size) == SOCK_EMPTY) {
            SocketError err = GetSocketError();

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

        if (::listen(sfd_, max_pending) == SOCK_EMPTY) {
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

        sfd_ = ::socket(family, type, protocol);

        if (sfd_ == SOCK_EMPTY && ip_type == IP::Type::ANY) {
            ip_type = IP::Type::V4;
            family  = AF_INET;
            sfd_    = ::socket(family, type, protocol);
        }

        ERR_FAIL_COND_V(sfd_ == SOCK_EMPTY, Error::FAILED);

        ip_type_ = ip_type;

        if (family == AF_INET6) {
            SetIpv6OnlyEnabled(ip_type != IP::Type::ANY);
        }

        if (protocol == IPPROTO_UDP && ip_type != IP::Type::V6) {
            SetBroadcastingEnabled(true);
        }

        is_stream_ = sock_type == SocketType::TCP;

        efd_ = ::epoll_create(EPOLL_MAX_EVENTS);

        if (efd_ == EPOLL_ERROR_ON_CREATE) {
            return Error::CANT_CREATE;
        }

        event_.data.fd = sfd_;
        event_.events  = EPOLLIN | EPOLLET;

        ctl_ = ::epoll_ctl(efd_, EPOLL_CTL_ADD, sfd_, &event_);

        if (ctl_ == EPOLL_ERROR_ON_CTL) {
            return Error::CANT_CREATE;
        }

        events_ = reinterpret_cast<struct epoll_event *>(::calloc(EPOLL_MAX_EVENTS, sizeof(event_)));

        return Error::OK;
    }

    Error
    Socket::Wait()
    {
        auto n = ::epoll_wait(efd_, events_, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);

        if (n == EPOLL_ERROR_ON_WAIT) {
            return Error::ERROR;
        }

        for (auto i = 0; i < n; ++i) {
            if (events_[i].events & (EPOLLERR | EPOLLHUP | EPOLLIN)) {
                return Error::ERROR;
            }

            if (events_[i].data.fd == sfd_) {
                return Error::OK;
            }
        }

        return Error::OK;
    }

    Error
    Socket::Recv(uint8_t &buffer, size_t len, ssize_t &bytes_read)
    {
        ERR_FAIL_COND_V(!IsOpen(), Error::ERR_UNCONFIGURED)

        bytes_read = ::recv(sfd_, &buffer, len, 0);

        if (bytes_read < 0) {
            SocketError err = GetSocketError();

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
            ::recvfrom(sfd_, &(buffer.at(0)), buffer.size(), 0, reinterpret_cast<struct sockaddr *>(&addr), &len);

        if (bytes_read < 0) {
            SocketError err = GetSocketError();

            if (err == SocketError::ERR_NET_WOULD_BLOCK) {
                return Error::ERR_BUSY;
            }

            return Error::FAILED;
        }

        if (addr.ss_family == AF_INET) {
            auto sin = reinterpret_cast<struct sockaddr_in *>(&addr);
            ip.SetIpv4(SPLIT_IPV4_TO_OCTET_INIT_LIST(sin->sin_addr));
            port = ntohs(sin->sin_port);
        }
        else if (addr.ss_family == AF_INET6) {
            auto sin6 = reinterpret_cast<struct sockaddr_in6 *>(&addr);
            ip.SetIpv6(sin6->sin6_addr.s6_addr);
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

        bytes_sent = ::send(sfd_, &buffer, len, flags);

        if (bytes_sent < 0) {
            SocketError err = GetSocketError();

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
        size_t addr_size = SetAddrStorage(addr, ip, port, ip_type_);

        bytes_sent = ::sendto(sfd_, buffer, len, 0, reinterpret_cast<struct sockaddr *>(&addr), addr_size);

        if (bytes_sent < 0) {
            SocketError err = GetSocketError();

            if (err == SocketError::ERR_NET_WOULD_BLOCK) {
                return Error::ERR_BUSY;
            }

            return Error::FAILED;
        }

        return Error::OK;
    }

    int
    Socket::AvailableBytes() const
    {
        ERR_FAIL_COND_V(sfd_ == SOCK_EMPTY, -1);

        int len;
        auto ret = ::ioctl(sfd_, FIONREAD, &len);

        ERR_FAIL_COND_V(ret == -1, 0)

        return len;
    }

    void
    Socket::SetBlockingEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());

        int ret  = 0;
        int opts = ::fcntl(sfd_, F_GETFL);

        if (enabled) {
            ret = ::fcntl(sfd_, F_SETFL, opts & ~O_NONBLOCK);
        }
        else {
            ret = ::fcntl(sfd_, F_SETFL, opts | O_NONBLOCK);
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

        if (setsockopt(sfd_, SOL_SOCKET, SO_BROADCAST, &par, sizeof(par)) != 0) {
            WARN_PRINT("Unable to change broadcast setting");
        }
    }

    void
    Socket::SetIpv6OnlyEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());
        ERR_FAIL_COND(ip_type_ == IP::Type::V4);

        auto par = enabled ? 1 : 0;

        if (setsockopt(sfd_, IPPROTO_IPV6, IPV6_V6ONLY, &par, sizeof(int)) != 0) {
            WARN_PRINT("Unable to change IPv4 address mapping over IPv6 option");
        }
    }

    void
    Socket::SetReuseAddressEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());

        auto par = enabled ? 1 : 0;

        if (setsockopt(sfd_, SOL_SOCKET, SO_REUSEADDR, &par, sizeof(int)) < 0) {
            WARN_PRINT("Unable to socket REUSEADDR option")
        }
    }

    void
    Socket::SetReusePortEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());

        auto par = enabled ? 1 : 0;

        if (setsockopt(sfd_, SOL_SOCKET, SO_REUSEPORT, &par, sizeof(int)) < 0) {
            WARN_PRINT("Unable to set socket REUSEPORT option");
        }
    }

    void
    Socket::SetTcpNoDelayEnabled(bool enabled)
    {
        ERR_FAIL_COND(!IsOpen());
        ERR_FAIL_COND(!is_stream_);

        auto par = enabled ? 1 : 0;

        if (setsockopt(sfd_, IPPROTO_TCP, TCP_NODELAY, &par, sizeof(int)) < 0) {
            ERR_PRINT("Unable to set TCP no delay option");
        }
    }
} // namespace core
