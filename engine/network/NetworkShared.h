#ifndef LIFE_NETWORKSHARED_H
#define LIFE_NETWORKSHARED_H

#include <netdb.h>

namespace engine
{
  namespace network {

    using Socket = int;

    const int INVALID_SOCKET = -1;
    const int NO_ERROR = 0;
    const int SOCKET_ERROR = -1;

    enum class SocketAddressFamily {
      INET = AF_INET,
      INET6 = AF_INET6
    };
  }
}

#endif // LIFE_NETWORKSHARED_H
