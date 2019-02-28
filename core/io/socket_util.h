#ifndef P2P_TECHDEMO_CORE_IO_SOCKET_UTIL_H
#define P2P_TECHDEMO_CORE_IO_SOCKET_UTIL_H

class SocketUtil
{
public:
    static bool is_connection_reset_on_recv(ssize_t read_byte_count);

    static bool is_no_messages_to_read(ssize_t read_byte_count);

    static int create_event_queue();

    static TCPSocketPtr create_tcp_socket(/* some arguments may be required */);

    static UDPSocketPtr create_udp_socket(/* some arguments may be required */);

    static wait_for_accepting(int fd, const std::vector <TCPSocketPtr> &in_sockets, std::vector <TCPSocketPtr> &out_sockets);

    static wait_for_receiving(int fd, const std::vector <TCPSocketPtr> &in_sockets, std::vector <TCPSocketPtr> &out_sockets);

    static Error register_event(int fd, const TCPSocketPtr &socket);

    static int last_error();
};

#endif // P2P_TECHDEMO_CORE_IO_SOCKET_UTIL_H
