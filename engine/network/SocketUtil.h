#ifndef LIFE_SOCKETUTIL_H
#define LIFE_SOCKETUTIL_H

#include <memory>

#include <spdlog/spdlog.h>

#include "TCPSocket.h"

namespace engine {
  namespace network {
    class SocketUtil {
    public:
      static bool static_init();

      static void report_error(const char *in_operation_desc);

      static int last_error();

      // static int select();

      // static UDPSocketPtr create_udp_socket(SocketAddressFamily in_family);

      static TCPSocketPtr create_tcp_socket(engine::network::SocketAddressFamily in_family);

//      static int wait(std::vector<TCPSocketPtr> &in_sockets, std::vector<TCPSocketPtr> &out_sockets);

      static int create_multiplexer();

      static void add_event(int mux, TCPSocketPtr socket);

      static int wait(int mux, const std::vector<TCPSocketPtr> &in_sockets, std::vector<TCPSocketPtr> &out_sockets);

    private:
      // inline static fd_set *fill_set_from_vector();

      // inline static void fill_vector_from_set();

      static std::shared_ptr<spdlog::logger> logger_;
    };
  } // namespace network
} // namespace engine

#endif // LIFE_SOCKETUTIL_H
