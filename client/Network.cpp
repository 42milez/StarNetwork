#include <cstddef>
#include <future>

#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace client
{
  using SocketAddress = engine::network::SocketAddress;
  using SocketAddressFactory = engine::network::SocketAddressFactory;
  using SocketUtil = engine::network::SocketUtil;
  using TCPSocketPtr = engine::network::TCPSocketPtr;

  bool Network::init() {
    return true;
  }

  std::vector<TCPSocketPtr> Network::wait() {
    std::vector<TCPSocketPtr> in_sockets{ tcp_socket_ };
    std::vector<TCPSocketPtr> out_sockets;

    engine::network::SocketUtil::wait_for_receiving(mux_, in_sockets, out_sockets);

    return std::move(out_sockets);
  }

} // namespace client
