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

  Network::Network() {
    logger_ = spdlog::basic_logger_mt("client / Network", "logs/development.log");
  }

  bool Network::init() {
    return true;
  }

  std::string Network::token_request(const std::string &id, const std::string &pw) {
    mux_ = engine::network::SocketUtil::create_multiplexer();

    // ソケットを生成
    auto tcp_socket = SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    // イベント登録
    SocketUtil::add_event(mux_, tcp_socket);

    SocketAddress client_address(INADDR_ANY, 42001);
    tcp_socket->bind(client_address);

    // サーバーに接続
    auto server_ip = SocketAddressFactory::create_ipv4_from_string("192.168.1.100:8888");
    SocketAddress server_addrss(*server_ip);
    tcp_socket->connect(server_addrss);

    std::string dummy_data{"hello"};

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

  std::vector<TCPSocketPtr> Network::wait() {
    std::vector<TCPSocketPtr> in_sockets{ tcp_socket_ };
    std::vector<TCPSocketPtr> out_sockets;

    engine::network::SocketUtil::wait(mux_, in_sockets, out_sockets);

    return std::move(out_sockets);
  }

} // namespace client
