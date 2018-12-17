#include <filesystem>
#include <iostream>

#include <spdlog/spdlog.h>

#include <engine/base/Singleton.h>
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
    do_run_loop();
  }

  void AuthServer::set_should_keep_running(bool should_keep_running) {
    should_keep_running_ = should_keep_running;
  }

  void AuthServer::do_run_loop() {
    auto network = engine::base::Singleton<Network>::Instance();
    while (should_keep_running_) {
      auto sockets_ready = network.wait();

      char buffer[1500];

      for (const auto &socket : sockets_ready) {
        auto bytes_received_count = socket->recv(buffer, sizeof(buffer));
        buffer[bytes_received_count] = '\0';
        std::cout << buffer << std::endl;
      }
    }
  }
} // namespace auth_server
