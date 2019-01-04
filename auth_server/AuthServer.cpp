#include <filesystem>
#include <iostream>

#include <spdlog/spdlog.h>

#include <engine/base/Singleton.h>
#include <engine/base/ExitHandler.h>
#include "auth_server/Network.h"

#include "AuthServer.h"

namespace auth_server
{
  AuthServer::AuthServer() : should_keep_running_(true) {
    // TODO Check the existence of the target file
    logger_ = spdlog::basic_logger_mt("auth_server / AuthServer", "logs/development.log");
  }

  bool AuthServer::init() {
    auto &network = engine::base::Singleton<Network>::Instance();
    network.init();

    return true;
  }

  void AuthServer::run() {
    engine::base::ExitHandler eh;
    auto &network = engine::base::Singleton<Network>::Instance();
    while (!eh.should_exit()) {
      auto error = network.process_incoming_packets();
    }
  }

  void AuthServer::set_should_keep_running(bool should_keep_running) {
    should_keep_running_ = should_keep_running;
  }
} // namespace auth_server
