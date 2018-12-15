#ifndef LIFE_NETWORK_H
#define LIFE_NETWORK_H

#include <memory>
#include <string>

#include "engine/network/NetworkBase.h"
#include "engine/network/SocketAddress.h"

namespace client {

  class Network : public network::NetworkBase {
  public:
    static std::unique_ptr<Network> instance_;
    static void static_init();
    std::string token_request(const std::string &id, const std::string &pw);
  private:
    void init();
  };

} // namespace client

#endif // LIFE_NETWORK_H
