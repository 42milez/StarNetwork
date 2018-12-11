#include <future>

#include <sys/event.h>
#include <tkDecls.h>

#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace client
{

  void Network::static_init(const network::SocketAddress &server_address) {
    instance_ = std::make_unique<Network>();
    return instance_->init(server_address);
  }

  std::string Network::token_request(const std::string &id, const std::string &pw) {
    std::promise<std::string> p;
    auto thread = std::thread([&p] {
      // ソケットを生成
      auto tcp_socket = network::SocketUtil::create_tcp_socket(network::SocketAddressFamily::INET);
      network::SocketAddress client_address(INADDR_ANY, 42001);
      tcp_socket->bind(client_address);

      // サーバーに接続
      auto server_ip = network::SocketAddressFactory::create_ipv4_from_string("192.168.0.1");
      network::SocketAddress server_addrss(*server_ip);
      tcp_socket->connect(server_addrss);

      std::string dummy_data{"hello"};

      // イベント登録
      network::SocketUtil::add_event(tcp_socket);

      // リクエスト送信
      tcp_socket->send(dummy_data.data(), sizeof(dummy_data.data()));

      // レスポンス待機
      std::vector<network::TCPSocketPtr> in_sockets{tcp_socket};
      std::vector<network::TCPSocketPtr> out_sockets;
      auto nfds = network::SocketUtil::wait(in_sockets, out_sockets);
      if (nfds == -1) {
        // error
      } else if (nfds == 0) {
        // timeout
      } else {
        // レスポンス受信（）
        size_t buffer_size = 1500;
        char buffer[buffer_size];
        tcp_socket->recv(buffer, buffer_size);
      }

      auto token = "";

      p.set_value(token);
    });

    p.get_future().wait();

    thread.join();

    return p.get_future().get();
  }

  void Network::init(const network::SocketAddress &server_address) {
    NetworkBase::init(0);
  }

} // namespace client
