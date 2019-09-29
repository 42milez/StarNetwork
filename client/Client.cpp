#include <ctime>
#include <iostream>
#include <string>
#include <core/singleton.h>

#include "engine/base/ExitHandler.h"
#include "engine/network/NetworkShared.h"
#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Client.h"
#include "Network.h"

namespace client
{
  using s_exit_handler = core::Singleton<engine::base::ExitHandler>;
  using s_network = core::Singleton<client::Network>;

  using SocketAddress = engine::network::SocketAddress;
  using SocketAddressFactory = engine::network::SocketAddressFactory;
  using SocketAddressPtr = engine::network::SocketAddressPtr;
  using SocketUtil = engine::network::SocketUtil;

  using SOCKET_STATUS = engine::network::SOCKET_STATUS;

  bool Client::init()
  {
    auto &eh = s_exit_handler::Instance();

    if (!eh.init()) return false;

    auto &network = s_network::Instance();

    if (!network.init()) return false;

    return true;
  }

  void Client::run()
  {
    auto &eh = s_exit_handler::Instance();

    while (!eh.should_exit()) {
      // ...
    }
  }

  int Client::request_token(const uint8_t *buf, uint32_t size)
  {
    auto mux = engine::network::SocketUtil::create_event_interface();

    auto tcp_socket = SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    if (tcp_socket == nullptr) {
      return SocketUtil::last_error();
    }

    // イベント登録
    SocketUtil::register_event(mux, tcp_socket);

    SocketAddress client_address{};
    tcp_socket->bind(client_address);

    auto server_address = SocketAddressFactory::create_ipv4_from_string("127.0.0.1:12345");
//    SocketAddress server_address(static_cast<uint32_t>(std::stoi("127.0.0.1")), 12345);
    auto error = tcp_socket->connect(*server_address);

    auto send_byte_count = tcp_socket->send(buf, size);

    if (static_cast<SOCKET_STATUS>(send_byte_count) == SOCKET_STATUS::FAIL) {
      SocketUtil::last_error();
      // TODO: Return appropriate status.
      return -1;
    }

    std::vector<TCPSocketPtr> in_sockets{tcp_socket};
    std::vector<TCPSocketPtr> out_sockets;

    for (;;) {
      auto nfds = SocketUtil::wait_for_receiving(mux, in_sockets, out_sockets);

      if (nfds == -1) {
        return -1;
      } else if (nfds == 0) {
        // timeout
        // retry
        // TODO: retry count
        // ...
        continue;
      } else {
        // レスポンス受信
        size_t buffer_size = 1500;
        char buffer[buffer_size];
        memset(buffer, 0, buffer_size);
        auto bytes_received_count = out_sockets[0]->recv(buffer, buffer_size);

        if (bytes_received_count < 0) {
          return -1;
        } else if (bytes_received_count == 0) {
          // peer closed
          // ...
        } else {
          buffer[bytes_received_count] = '\0';
          token_ = std::string(buffer);
          break;
        }
      }
    }

    return 0;
  }

  bool Client::token_exists() {
    return !token_.empty();
  }

} // namespace client
