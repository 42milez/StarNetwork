#ifndef P2P_TECHDEMO_CORE_IO_SOCKET_H
#define P2P_TECHDEMO_CORE_IO_SOCKET_H

#include <memory>

#include "core/base/errors.h"
#include "ip.h"

class Socket;

using SOCKET = int;
using SOCKET_PTR =  std::shared_ptr<Socket>;

class Socket
{
private:
    SOCKET _sock;

    IP::Type _ip_type;

    bool _is_stream;

private:
    bool _can_use_ip(const IpAddress &ip_addr, bool for_bind) const;

    socklen_t _set_addr_storage(struct sockaddr_storage &addr, const IpAddress &ip, uint16_t port, IP::Type ip_type);

    void _set_ip_port(struct sockaddr_storage &addr, IpAddress &ip, uint16_t &port);

    void _set_socket(SOCKET sock, IP::Type ip_type, bool is_stream);

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

    SOCKET_PTR accept(IpAddress &ip, uint16_t port);

    Error bind(const IpAddress &ip, uint16_t port);

    void close();

    Error connect(const IpAddress &ip, uint16_t port);

    Error listen(int max_pending);

    Error open(Type p_type, IP::Type ip_type);

    Error poll(PollType type, int timeout);

    Error recv(uint8_t &buffer, int len, int &read_byte_count);

    Error recvfrom(uint8_t &buffer, int len, int &read_byte_count);

    Error send(const uint8_t &buffer, int len, int send_byte_count);

    Error sendto(const uint8_t &buffer, int len, int send_byte_count);

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
};

#endif // P2P_TECHDEMO_CORE_IO_SOCKET_H
