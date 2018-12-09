#ifndef LIFE_SOCKETADDRESSFACTORY_H
#define LIFE_SOCKETADDRESSFACTORY_H

#include "SocketAddress.h"

namespace network {

  class SocketAddressFactory {
  public:
    static SocketAddressPtr create_ipv4_from_string(const string &in_string);
  };

} // namespace network

#endif // LIFE_SOCKETADDRESSFACTORY_H
