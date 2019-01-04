#ifndef P2P_TECHDEMO_NETWORKSHARED_H
#define P2P_TECHDEMO_NETWORKSHARED_H

#include <netdb.h>

namespace engine
{
  namespace network {

    using Socket = int;

    const int NO_ERROR = 0;
    const int INVALID_SOCKET = -1;
    const int WSAECONNRESET = ECONNRESET;
    const int WSAEWOULDBLOCK = EAGAIN;
    const int SOCKET_ERROR = -1;

    enum class SocketAddressFamily {
      INET = AF_INET,
      INET6 = AF_INET6
    };
  }
}

#endif // P2P_TECHDEMO_NETWORKSHARED_H
