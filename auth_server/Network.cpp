#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace auth_server {
  namespace {
    using SocketAddress = engine::network::SocketAddress;
  }

  Network::Network() {
    logger_ = spdlog::basic_logger_mt("auth_server / Network", "logs/development.log");
  }

  bool Network::init() {
    server_socket_ = engine::network::SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    if (server_socket_ == nullptr) {
      return false;
    }

    SocketAddress server_address(INADDR_ANY, 12345);

    // TODO: Retry when bind() fails
    server_socket_->bind(server_address);

    server_socket_->listen(5);

    mux_ = engine::network::SocketUtil::create_multiplexer();

    auto error = engine::network::SocketUtil::add_event(mux_, server_socket_);

    return true;
  }

  std::vector<TCPSocketPtr> Network::wait() {
    std::vector<TCPSocketPtr> in_sockets{server_socket_};
    std::vector<TCPSocketPtr> out_sockets;

    engine::network::SocketUtil::wait_for_accepting(mux_, in_sockets, out_sockets);

    return std::move(out_sockets);
  }
} // namespace auth_server
