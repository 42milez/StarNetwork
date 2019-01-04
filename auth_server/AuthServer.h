#ifndef P2P_TECHDEMO_AUTHSERVER_AUTHSERVER_H
#define P2P_TECHDEMO_AUTHSERVER_AUTHSERVER_H

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
    bool should_keep_running_;

    std::shared_ptr<spdlog::logger> logger_;
  };
} // namespace auth_server

#endif // P2P_TECHDEMO_AUTHSERVER_AUTHSERVER_H
