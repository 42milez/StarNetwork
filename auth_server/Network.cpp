#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace auth_server
{
  Network::Network() {
    logger_ = spdlog::basic_logger_mt("auth_server / Network", "logs/development.log");
  }

  bool Network::init() {
    tcp_socket_ = engine::network::SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    if (tcp_socket_ == nullptr) {
      return false;
    }

    auto ipv4 = engine::network::SocketAddressFactory::create_ipv4_from_string("192.168.1.100:8888");

    engine::network::SocketAddress tcp_address(*ipv4);

    tcp_socket_->bind(tcp_address);

    tcp_socket_->listen(5);

    mux_ = engine::network::SocketUtil::create_multiplexer(tcp_socket_);

    return true;
  }

  std::vector<TCPSocketPtr> Network::wait() {
    std::vector<TCPSocketPtr> in_sockets{ tcp_socket_ };
    std::vector<TCPSocketPtr> out_sockets;

    engine::network::SocketUtil::wait(mux_, in_sockets, out_sockets);

    return std::move(out_sockets);
  }
} // namespace auth_server
