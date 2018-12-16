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

  void Network::static_init() {
    instance_ = std::make_unique<Network>();
    return instance_->init();
  }

  std::string Network::token_request(const std::string &id, const std::string &pw) {
    // ソケットを生成
    auto tcp_socket = SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);
    SocketAddress client_address(INADDR_ANY, 42001);
    tcp_socket->bind(client_address);

    // サーバーに接続
    auto server_ip = SocketAddressFactory::create_ipv4_from_string("192.168.0.1");
    SocketAddress server_addrss(*server_ip);
    tcp_socket->connect(server_addrss);

    std::string dummy_data{"hello"};

    // イベント登録
    SocketUtil::add_event(tcp_socket);

    // リクエスト送信
    tcp_socket->send(dummy_data.data(), sizeof(dummy_data.data()));

    // レスポンス待機
    std::string token = nullptr;
    std::vector<TCPSocketPtr> in_sockets{tcp_socket};
    std::vector<TCPSocketPtr> out_sockets;

    auto nfds = SocketUtil::wait(in_sockets, out_sockets);

    if (nfds == -1) {
      // error
    } else if (nfds == 0) {
      // timeout
    } else {
      // レスポンス受信
      size_t buffer_size = 1500;
      char buffer[buffer_size];
      memset(buffer, 0, buffer_size);
      auto bytes_received_count = out_sockets[0]->recv(buffer, buffer_size);
      buffer[bytes_received_count] = '\0';
      token = std::string(buffer);
    }

    return token;
  }

  void Network::init() {
    NetworkBase::init(0);
  }

} // namespace client
