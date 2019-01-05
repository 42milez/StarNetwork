#ifndef P2P_TECHDEMO_NETWORK_H
#define P2P_TECHDEMO_NETWORK_H

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include "engine/network/SocketAddress.h"
#include "engine/network/TCPSocket.h"

namespace client
{
  using engine::network::TCPSocketPtr;

  class Network {
  public:
    bool init();

    std::vector<TCPSocketPtr> wait();

  private:
    std::shared_ptr<spdlog::logger> logger_;

    engine::network::TCPSocketPtr tcp_socket_;

    int mux_;
  };

} // namespace client

#endif // P2P_TECHDEMO_NETWORK_H
