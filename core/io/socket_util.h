#ifndef P2P_TECHDEMO_CORE_IO_SOCKET_UTIL_H
#define P2P_TECHDEMO_CORE_IO_SOCKET_UTIL_H

#include <memory>

#include "ip.h"
#include "socket.h"

class SocketUtil
{
public:
    static bool is_connection_reset_on_recv(ssize_t read_byte_count);

    static bool is_no_messages_to_read(ssize_t read_byte_count);

    static int create_event_queue();

    static SOCKET_PTR create_socket(Socket::Type sock_type, IP::Type ip_type);

    static wait_for_accepting(int fd, const std::vector<SOCKET_PTR> &in_sockets, std::vector<SOCKET_PTR> &out_sockets);

    static wait_for_receiving(int fd, const std::vector<SOCKET_PTR> &in_sockets, std::vector<SOCKET_PTR> &out_sockets);

    static Error register_event(int fd, const SOCKET_PTR &socket);

    static int last_error();
};

#endif // P2P_TECHDEMO_CORE_IO_SOCKET_UTIL_H
