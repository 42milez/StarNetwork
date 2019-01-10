#ifndef P2P_TECHDEMO_NETWORKSHARED_H
#define P2P_TECHDEMO_NETWORKSHARED_H

#include <netdb.h>

namespace engine
{
  namespace network {

    using Socket = int;

    const int NO_ERROR = 0;
    const int ERROR = -1;
    const int INVALID_SOCKET = -1;
    const int WSAECONNRESET = ECONNRESET;
    const int WSAEWOULDBLOCK = EAGAIN;
    const int SOCKET_ERROR = -1;
    const int FAIL_CREATE_EVENT_PIPELINE = -1;

    enum class SocketAddressFamily {
      INET = AF_INET,
      INET6 = AF_INET6
    };

    enum class SOCKET_STATUS {
      SUCCESS = 0,
      FAIL = -1
    };

    enum class KEVENT_STATUS {
      TIMEOUT = 0,
      FAIL = -1
    };

    enum class KQUEUE_STATUS {
      FAIL = -1
    };
  }
}

#endif // P2P_TECHDEMO_NETWORKSHARED_H
