#include <ctime>
#include <string>
#include <engine/base/Singleton.h>

#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Auth.h"
#include "Client.h"
#include "Network.h"

namespace client
{
  using SocketAddressFactory = engine::network::SocketAddressFactory;
  using SocketAddressPtr = engine::network::SocketAddressPtr;

  bool Client::init() {
    auto auth = engine::base::Singleton<auth::Auth>::Instance();
    auth.init();

    auto network = engine::base::Singleton<client::Network>::Instance();
    network.init();

    return true;
  }

  Client::Client() : should_keep_running_(true) {
    // ...
  }

  std::string Client::request_token() {
    return "";
  }

  void Client::run() {
    do_run_loop();
  }

  void Client::set_should_keep_running(bool should_keep_running) {
    should_keep_running_ = should_keep_running;
  }

  void Client::do_run_loop() {
    while (should_keep_running_) {
      do_frame();
    }
  }

  void Client::do_frame() {
    // ...
  }

} // namespace client
