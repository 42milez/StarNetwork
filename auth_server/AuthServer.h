#ifndef P2P_TECHDEMO_AUTHSERVER_AUTHSERVER_H
#define P2P_TECHDEMO_AUTHSERVER_AUTHSERVER_H

#include <memory>

#include <spdlog/spdlog.h>

namespace auth_server
{
  class AuthServer {
  public:
    bool init();

    void run();
  };
} // namespace auth_server

#endif // P2P_TECHDEMO_AUTHSERVER_AUTHSERVER_H