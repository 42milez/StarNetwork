#ifndef LIFE_NETWORKBASE_H
#define LIFE_NETWORKBASE_H

#include <cstdint>

#include <spdlog/spdlog.h>

#include "engine/network/TCPSocket.h"

namespace engine
{
  namespace network
  {
    class NetworkBase {
    public:
      NetworkBase();
      bool init(uint16_t port);
    private:
      std::shared_ptr<spdlog::logger> logger_;
      ::network::TCPSocketPtr tcp_socket_;
    };

  } // namespace network
} // namespace engine

#endif // LIFE_NETWORKBASE_H
