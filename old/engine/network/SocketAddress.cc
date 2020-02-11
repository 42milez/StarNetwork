#include "SocketAddress.h"

namespace engine
{
  namespace network {

    SocketAddress::SocketAddress(uint32_t address, uint16_t port) {
      as_sockaddr_in()->sin_family = AF_INET;
      ip4_ref() = htonl(address);
      as_sockaddr_in()->sin_port = htons(port);
    }

    SocketAddress::SocketAddress(const sockaddr& sockaddr) {
      memcpy(&sockaddr_, &sockaddr, sizeof(sockaddr));
    }

    SocketAddress::SocketAddress() {
      as_sockaddr_in()->sin_family = AF_INET;
      ip4_ref() = INADDR_ANY;
      as_sockaddr_in()->sin_port = 0;
    }

    uint32_t SocketAddress::size() const {
      return sizeof(sockaddr_);
    }

    uint32_t &SocketAddress::ip4_ref() {
      return as_sockaddr_in()->sin_addr.s_addr;
    }

//  const uint32_t &SocketAddress::ip4_ref() const {
//    return as_sockaddr_in()->sin_addr.s_addr;
//  }

    sockaddr_in *SocketAddress::as_sockaddr_in() {
      return reinterpret_cast<sockaddr_in *>(&sockaddr_);
    }

//  const sockaddr_in *SocketAddress::as_sockaddr_in() const {
//    return reinterpret_cast<const sockaddr_in *>(&sockaddr_);
//  }

  } // namespace network
}
