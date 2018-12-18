#include <ctime>
#include <iostream>
#include <string>
#include <engine/base/Singleton.h>

#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Client.h"
#include "Network.h"

namespace client
{
  using SocketAddress = engine::network::SocketAddress;
  using SocketAddressFactory = engine::network::SocketAddressFactory;
  using SocketAddressPtr = engine::network::SocketAddressPtr;
  using SocketUtil = engine::network::SocketUtil;

  bool Client::init() {
    auto &network = engine::base::Singleton<client::Network>::Instance();
    network.init();

    return true;
  }

  Client::Client() : should_keep_running_(true) {
    // ...
  }

  void Client::run() {
    do_run_loop();
  }

  void Client::set_should_keep_running(bool should_keep_running) {
    should_keep_running_ = should_keep_running;
  }

  void Client::do_run_loop() {
    while (should_keep_running_) {
      do_frame();
    }
  }

  void Client::do_frame() {
    // ...
  }

  int Client::request_token(const std::string &id, const std::string &pw) {
    auto mux = engine::network::SocketUtil::create_multiplexer();

    // ソケットを生成
    auto tcp_socket = SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    // イベント登録
    SocketUtil::add_event(mux, tcp_socket);

    SocketAddress client_address{};
    tcp_socket->bind(client_address);

    // サーバーに接続
    auto server_address = SocketAddressFactory::create_ipv4_from_string("127.0.0.1:12345");
//    SocketAddress server_address(static_cast<uint32_t>(std::stoi("127.0.0.1")), 12345);
    auto error = tcp_socket->connect(*server_address);

    std::string dummy_data{"hello"};

    // リクエスト送信
    auto size = tcp_socket->send(dummy_data.data(), sizeof(dummy_data.data()));

    // レスポンス待機
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
          std::cout << "a" << std::endl;
          continue;
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
