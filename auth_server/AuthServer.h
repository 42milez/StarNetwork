#ifndef LIFE_AUTHSERVER_AUTHSERVER_H
#define LIFE_AUTHSERVER_AUTHSERVER_H

#include <memory>

#include <spdlog/spdlog.h>

namespace auth_server
{
  class AuthServer {
  public:
    AuthServer();

    bool init();

    void run();

  private:
    void set_should_keep_running(bool should_keep_running);

    void do_run_loop();

    bool should_keep_running_;

    std::shared_ptr<spdlog::logger> logger_;
  };
} // namespace auth_server

#endif // LIFE_AUTHSERVER_AUTHSERVER_H
