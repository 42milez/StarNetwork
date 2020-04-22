#ifndef P2P_TECHDEMO_LIB_CORE_IO_SOCKET_UTIL_H_
#define P2P_TECHDEMO_LIB_CORE_IO_SOCKET_UTIL_H_

#include <memory>

#include "ip.h"
#include "socket.h"

namespace core
{
    class SocketUtil
    {
      public:
        static bool
        is_connection_reset_on_recv(ssize_t bytes_read);

        static bool
        is_no_messages_to_read(ssize_t bytes_read);

        // static int create_event_queue();

        static std::shared_ptr<Socket>
        create_socket();

        // static void wait_for_accepting(int fd, const std::vector<SOCKET_PTR> &in_sockets, std::vector<SOCKET_PTR>
        // &out_sockets);

        // static void wait_for_receiving(int fd, const std::vector<SOCKET_PTR> &in_sockets, std::vector<SOCKET_PTR>
        // &out_sockets);

        // static Error register_event(int fd, const SOCKET_PTR &socket);

        static int
        last_error();
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_IO_SOCKET_UTIL_H_
