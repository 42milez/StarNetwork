#ifndef P2P_TECHDEMO_SOCKETUTIL_H
#define P2P_TECHDEMO_SOCKETUTIL_H

#include <map>
#include <memory>

#include <spdlog/spdlog.h>

#include "TCPSocket.h"

namespace engine
{
  namespace network
  {
    class SocketUtil {
    public:
      static bool static_init();

      static void report_error(const char *in_operation_desc);

      static int last_error();

      // static int select();

      // static UDPSocketPtr create_udp_socket(SocketAddressFamily in_family);

      static TCPSocketPtr create_tcp_socket(engine::network::SocketAddressFamily in_family);

//      static int wait(std::vector<TCPSocketPtr> &in_sockets, std::vector<TCPSocketPtr> &out_sockets);

      static int create_event_interface();

      static void add_socket(std::map<int, TCPSocketPtr> &sockets, const TCPSocketPtr &socket);

      static KEVENT_STATUS register_event(int mux, const TCPSocketPtr &socket);

      static int wait_for_accepting(int mux, const std::vector<TCPSocketPtr> &in_sockets, std::vector<TCPSocketPtr> &out_sockets);
      static int wait_for_receiving(int mux, const std::vector<TCPSocketPtr> &in_sockets, std::vector<TCPSocketPtr> &out_sockets);

      static bool is_connection_reset_on_recv(ssize_t read_byte_count);
      static bool is_no_messages_to_read(ssize_t read_byte_count);

    private:
      // inline static fd_set *fill_set_from_vector();

      // inline static void fill_vector_from_set();
    };
  } // namespace network
} // namespace engine

#endif // P2P_TECHDEMO_SOCKETUTIL_H
