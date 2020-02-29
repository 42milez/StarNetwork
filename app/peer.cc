#include "lib/core/singleton.h"
#include "lib/core/ExitHandler.h"
#include "network.h"
#include "peer.h"

namespace peer
{
  using s_exit_handler = core::Singleton<core::ExitHandler>;
  using s_network = core::Singleton<peer::Network>;

  bool Peer::Init()
  {
    auto &eh = s_exit_handler::Instance();

    if (!eh.init()) return false;

    auto &network = s_network::Instance();

    if (!network.Init()) return false;

    return true;
  }

  void Peer::Run()
  {
    auto &eh = s_exit_handler::Instance();

    while (!eh.should_exit()) {
      // ...
    }
  }
} // namespace peer
