#ifndef P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H
#define P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H

#include <sys/socket.h>

#include "core/io/socket.h"
#include "ip_unix.h"

using SOCKET = int;

class SocketUnix : public core::io::Socket
{
private:
    SOCKET _sock;

    core::io::IP::Type _ip_type;

    bool _is_stream;

    enum class NetError : int
    {
        ERR_NET_WOULD_BLOCK,
        ERR_NET_IS_CONNECTED,
        ERR_NET_IN_PROGRESS,
        ERR_NET_OTHER
    };

    NetError _get_socket_error();

    void _set_socket(SOCKET sock, core::io::IP::Type ip_type, bool is_stream);

protected:
    bool _can_use_ip(core::io::IpAddress ip_addr, bool for_bind) const;
};

#endif // P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H
