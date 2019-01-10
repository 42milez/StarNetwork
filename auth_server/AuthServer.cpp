#include <engine/base/Singleton.h>
#include <engine/base/ExitHandler.h>
#include "auth_server/Network.h"

#include "AuthServer.h"

namespace auth_server
{
  bool AuthServer::init() {
    auto &network = engine::base::Singleton<Network>::Instance();
    return network.init();
  }

  void AuthServer::run() {
    engine::base::ExitHandler eh;
    auto &network = engine::base::Singleton<Network>::Instance();
    while (!eh.should_exit()) {
      network.process_incoming_packets();
    }
  }
} // namespace auth_server