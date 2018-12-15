#ifndef LIFE_SOCKETADDRESS_H
#define LIFE_SOCKETADDRESS_H

#include <cstddef>
#include <cstdint>

#include <memory>
#include <string>

#include <netinet/in.h>

using std::string;

namespace engine
{
  namespace network
  {
    class SocketAddress {
    public:
      SocketAddress(uint32_t address, uint16_t port);

      SocketAddress(const sockaddr &sockaddr);

      SocketAddress();

      bool operator==(const SocketAddress &other) const;

      size_t hash();

      uint32_t size() const;

      string to_string();

    private:
      friend class UDPSocket;

      friend class TCPSocket;

      sockaddr sockaddr_;

      uint32_t &ip4_ref();

      // const uint32_t &ip4_ref() const;

      sockaddr_in *as_sockaddr_in();

      // const sockaddr_in *as_sockaddr_in() const;
    };

    using SocketAddressPtr = std::shared_ptr<SocketAddress>;

  } // namespace network
} // namespace engine

#endif // LIFE_SOCKETADDRESS_H
