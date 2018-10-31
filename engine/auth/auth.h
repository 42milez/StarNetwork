#ifndef LIFE_AUTH_H
#define LIFE_AUTH_H

#include <memory>
#include <string>

#include "engine/network/tcp_socket.h"

namespace auth
{
  class Auth {
  public:
    Auth();

    void authorize(const std::string &id, const std::string &pw);

  private:
    std::string token;

    std::shared_ptr<network::TcpSocket> soc_;
  };
} // namespace auth

#endif // LIFE_AUTH_H
