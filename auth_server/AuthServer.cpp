#include <spdlog/spdlog.h>

#include "auth_server/Network.h"

#include "AuthServer.h"

namespace auth_server
{
  AuthServer::AuthServer() : should_keep_running_(true) {
    logger_ = spdlog::basic_logger_mt("auth_server / Network", "logs/development.log");
  }

  bool AuthServer::init() {
    return true;
  }

  void AuthServer::run() {
    do_run_loop();
  }

  void AuthServer::set_should_keep_running(bool should_keep_running) {
    should_keep_running_ = should_keep_running;
  }

  void AuthServer::do_run_loop() {
    while (should_keep_running_) {

    }
  }
} // namespace auth_server
