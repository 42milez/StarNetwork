#ifndef LIFE_NETWORK_H
#define LIFE_NETWORK_H

#include <memory>
#include <string>

#include "engine/network/NetworkBase.h"
#include "engine/network/SocketAddress.h"

namespace client
{
  using engine::network::TCPSocketPtr;

  class Network {
  public:
    Network();

    bool init();

    std::vector<TCPSocketPtr> wait();

    std::string token_request(const std::string &id, const std::string &pw);

  private:
    std::shared_ptr<spdlog::logger> logger_;

    engine::network::TCPSocketPtr tcp_socket_;

    int mux_;
  };

} // namespace client

#endif // LIFE_NETWORK_H
