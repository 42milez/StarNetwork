#ifndef LIFE_AUTHSERVER_NETWORK_H
#define LIFE_AUTHSERVER_NETWORK_H

#include "engine/network/TCPSocket.h"

namespace auth_server
{
  using TCPSocketPtr = engine::network::TCPSocketPtr;

  class Network {
  public:
    Network();

    bool init();

    std::vector<TCPSocketPtr> wait();

  private:
    std::shared_ptr<spdlog::logger> logger_;

    engine::network::TCPSocketPtr tcp_socket_;

    std::array<int, 1024> queue_;

    int mux_;
  };
} // namespace auth_server

#endif // LIFE_AUTHSERVER_NETWORK_H
