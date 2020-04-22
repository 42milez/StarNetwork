#ifndef P2P_TECHDEMO_LIB_CORE_IO_SOCKET_H_
#define P2P_TECHDEMO_LIB_CORE_IO_SOCKET_H_

#include <memory>

#include <sys/socket.h>
#include <unistd.h>

#include "ip.h"
#include "lib/core/errors.h"

namespace core
{
    enum class PollType
    {
        POLL_TYPE_IN,
        POLL_TYPE_OUT,
        POLL_TYPE_IN_OUT
    };

    enum class SocketType
    {
        NONE,
        TCP,
        UDP
    };

    constexpr int SOCK_EMPTY = -1;

    class Socket
    {
      public:
        Socket();
        Socket(int sock, IP::Type ip_type, bool is_stream);

        std::shared_ptr<Socket>
        Accept(IpAddress &ip, uint16_t &port);

        int
        AvailableBytes() const;

        Error
        Bind(const IpAddress &ip, uint16_t port);

        Error
        Connect(const IpAddress &ip, uint16_t port);

        void
        DisableBlocking();

        void
        EnableBroadcasting();

        Error
        Listen(int max_pending);

        Error
        Open(SocketType p_type, IP::Type ip_type);

        Error
        Poll(PollType type, int timeout);

        Error
        Recv(uint8_t &buffer, size_t len, ssize_t &bytes_read);

        Error
        RecvFrom(std::vector<uint8_t> &buffer, ssize_t &bytes_read, IpAddress &ip, uint16_t &port);

        Error
        Send(const uint8_t &buffer, size_t len, ssize_t &bytes_sent);

        Error
        SendTo(const void *buffer, size_t len, ssize_t &bytes_sent, const IpAddress &ip, uint16_t port);

      public:
        bool
        IsOpen() const;

      public:
        void
        set_ipv6_only_enabled(bool enabled);

        void
        set_reuse_address_enabled(bool enabled);

        void
        set_reuse_port_enabled(bool enabled);

        void
        set_tcp_no_delay_enabled(bool enabled);

        inline ~Socket()
        {
            Close();
        }

        inline void
        Close()
        {
            if (sock_ != SOCK_EMPTY) {
                ::close(sock_);
            }

            sock_      = SOCK_EMPTY;
            ip_type_   = IP::Type::NONE;
            is_stream_ = false;
        }

      private:
        int sock_;
        IP::Type ip_type_;
        bool is_stream_;
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_IO_SOCKET_H_
