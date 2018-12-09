#include <future>

#include <sys/event.h>

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

  std::string Network::send_token_request(const std::string &id, const std::string &pw) {
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

      // リクエスト送信
      tcp_socket->send(dummy_data.data(), sizeof(dummy_data.data()));

      // レスポンス受信（）
      tcp_socket->recv();

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
