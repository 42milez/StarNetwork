#ifndef P2P_TECHDEMO_CORE_IO_SOCKET_H
#define P2P_TECHDEMO_CORE_IO_SOCKET_H

#include <memory>

#include "core/base/errors.h"
#include "ip.h"

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

    virtual Error open(Type p_type, IP::Type ip_type) = 0;

    virtual void close() = 0;

    virtual Error bind(IpAddress &ip, uint16_t port) = 0;

    virtual Error listen(int max_pending) = 0;

    virtual Error connect_to_host(IpAddress &ip, uint16_t port) = 0;

    virtual Error poll(PollType type, int timeout) = 0;

    virtual Error recv(uint8_t &buffer, int len, int &read_byte_count) = 0;

    virtual Error recvfrom(uint8_t &buffer, int len, int &read_byte_count) = 0;

    virtual Error send(const uint8_t &buffer, int len, int send_byte_count) = 0;

    virtual Error sendto(const uint8_t &buffer, int len, int send_byte_count) = 0;

    virtual std::shared_ptr<Socket> accept(IpAddress &ip, uint16_t port) = 0;

    virtual bool is_open() const = 0;

    virtual int get_available_bytes() const = 0;

    virtual void set_broadcasting_enabled(bool enabled) = 0;

    virtual void set_blocking_enabled(bool enabled) = 0;

    virtual void set_ipv6_only_enabled(bool enabled) = 0;

    virtual void set_tcp_no_delay_enabled(bool enabled) = 0;

    virtual void set_reuse_address_enabled(bool enabled) = 0;
};

#endif // P2P_TECHDEMO_CORE_IO_SOCKET_H
