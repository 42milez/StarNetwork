#ifndef LIFE_AUTHSERVER_NETWORK_H
#define LIFE_AUTHSERVER_NETWORK_H

#include "engine/network/TCPSocket.h"

namespace auth_server
{
  class Network {
  public:
    Network();

    bool init();

    void wait_for_accept();

    static std::unique_ptr<Network> instance_;

  private:
    std::shared_ptr<spdlog::logger> logger_;

    engine::network::TCPSocketPtr tcp_socket_;

    std::array<int, 1024> queue_;
  };
} // namespace auth_server

#endif // LIFE_AUTHSERVER_NETWORK_H
