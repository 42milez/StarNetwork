#ifndef P2P_TECHDEMO_SOCKETADDRESSFACTORY_H
#define P2P_TECHDEMO_SOCKETADDRESSFACTORY_H

#include "SocketAddress.h"

namespace engine
{
  namespace network
  {
    class SocketAddressFactory {
    public:
      static SocketAddressPtr create_ipv4_from_string(const string &in_string);
    };
  } // namespace network
} // namespace engine

#endif // P2P_TECHDEMO_SOCKETADDRESSFACTORY_H
