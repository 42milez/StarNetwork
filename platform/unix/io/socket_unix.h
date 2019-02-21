#ifndef P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H
#define P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H

#include <sys/socket.h>

#include "core/io/socket.h"
#include "ip_unix.h"

using SOCKET = int;

class SocketUnix : public Socket
{
private:
    SOCKET _sock;

    IP::Type _ip_type;

    bool _is_stream;

    enum class NetError : int
    {
        ERR_NET_WOULD_BLOCK,
        ERR_NET_IS_CONNECTED,
        ERR_NET_IN_PROGRESS,
        ERR_NET_OTHER
    };

    NetError _get_socket_error();

    void _set_socket(SOCKET sock, IP::Type ip_type, bool is_stream);

protected:
    bool _can_use_ip(IpAddress ip_addr, bool for_bind) const;

public:
    size_t _set_addr_storage(struct sockaddr_storage &addr, const IpAddress &ip, uint16_t port, IP::Type ip_type);

    void _set_ip_port(struct sockaddr_storage &addr, IpAddress &ip, uint16_t &port);

    void set_ipv6_only_enabled(bool enabled) override;

    void set_reuse_address_enabled(bool enabled) override;

    void set_reuse_port_enabled(bool enabled);

    void set_tcp_no_delay_enabled(bool enabled) override;

    void close();

    Error open(Type sock_type, IP::Type ip_type) override;

    SocketUnix();
    ~SocketUnix();
};

#endif // P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H
