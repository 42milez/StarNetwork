#ifndef P2P_TECHDEMO_CORE_IO_SOCKET_H
#define P2P_TECHDEMO_CORE_IO_SOCKET_H

#include <memory>

#include "core/base/errors.h"
#include "ip.h"

using SOCKET = int;

class Socket
{
public:
    enum class PollType
    {
        POLL_TYPE_IN,
        POLL_TYPE_OUT,
        POLL_TYPE_IN_OUT
    };

    enum class Type
    {
        NONE,
        TCP,
        UDP
    };

public:
    socklen_t _set_addr_storage(struct sockaddr_storage &addr, const IpAddress &ip, uint16_t port, IP::Type ip_type);

    void _set_ip_port(struct sockaddr_storage &addr, IpAddress &ip, uint16_t &port);

    Error open(Type p_type, IP::Type ip_type);

    void close();

    Error bind(const IpAddress &ip, uint16_t port);

    Error listen(int max_pending);

    Error connect(const IpAddress &ip, uint16_t port);

    Error poll(PollType type, int timeout);

    Error recv(uint8_t &buffer, int len, int &read_byte_count);

    Error recvfrom(uint8_t &buffer, int len, int &read_byte_count);

    Error send(const uint8_t &buffer, int len, int send_byte_count);

    Error sendto(const uint8_t &buffer, int len, int send_byte_count);

    std::shared_ptr<Socket> accept(IpAddress &ip, uint16_t port);

    bool is_open() const;

    int get_available_bytes() const;

    void set_blocking_enabled(bool enabled);

    void set_broadcasting_enabled(bool enabled);

    void set_ipv6_only_enabled(bool enabled);

    void set_reuse_address_enabled(bool enabled);

    void set_reuse_port_enabled(bool enabled);

    void set_tcp_no_delay_enabled(bool enabled);

    Socket();

    ~Socket();

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

private:
    bool _can_use_ip(const IpAddress &ip_addr, bool for_bind) const;

    NetError _get_socket_error();

    void _set_socket(SOCKET sock, IP::Type ip_type, bool is_stream);
};

#endif // P2P_TECHDEMO_CORE_IO_SOCKET_H
