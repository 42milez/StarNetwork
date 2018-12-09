#include "NetworkBase.h"

#include "engine/network/NetworkShared.h"
#include "engine/network/SocketAddress.h"
#include "engine/network/SocketUtil.h"

namespace network {

  NetworkBase::NetworkBase() {
    logger_ = spdlog::stdout_color_mt("NetworkBase");
  }

  bool NetworkBase::init(uint16_t port) {
    tcp_socket_ = SocketUtil::create_tcp_socket(SocketAddressFamily::INET);
    SocketAddress tcp_address(INADDR_ANY, port);
    tcp_socket_->bind(tcp_address);

    logger_->info("Initializing NetworkBase at port {0} (TCP)", port);

    if(tcp_socket_ == nullptr) {
      return false;
    }

    return true;
  }

} // namespace network
