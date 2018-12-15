#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace auth_server
{
  Network::Network() {
    logger_ = spdlog::basic_logger_mt("auth_server / Network", "logs/development.log");
  }

  bool Network::init() {
    tcp_socket_ = ::network::SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    if (tcp_socket_ == nullptr) {
      return false;
    }

    auto ipv4 = engine::network::SocketAddressFactory::create_ipv4_from_string("192.168.1.100:8888");

    engine::network::SocketAddress tcp_address(*ipv4);

    tcp_socket_->bind(tcp_address);

    return true;
  }

  void Network::wait_for_accept() {

  }
} // namespace auth_server
