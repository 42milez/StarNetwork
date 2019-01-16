#include <engine/base/Singleton.h>
#include <engine/base/ExitHandler.h>

#include "auth_server/Network.h"

#include "AuthServer.h"

namespace auth_server
{
  namespace
  {
    using s_network = engine::base::Singleton<Network>;
    using s_exit_handler = engine::base::Singleton<engine::base::ExitHandler>;
  }

  bool
  AuthServer::init()
  {
    auto &eh = s_exit_handler::Instance();

    if (!eh.init()) return false;

    auto &network = s_network::Instance();

    if (!network.init()) return false;

    return true;
  }

  void
  AuthServer::run()
  {
    auto &eh = s_exit_handler::Instance();
    auto &network = s_network::Instance();

    while (!eh.should_exit())
    {
      network.process_incoming_packets();
    }
  }
} // namespace auth_server
