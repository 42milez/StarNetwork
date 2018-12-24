#include <iostream>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "engine/network/NetworkShared.h"
#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace auth_server {
  namespace {
    using SocketAddress = engine::network::SocketAddress;
    using SocketUtil = engine::network::SocketUtil;
  }

  Network::Network() {
    logger_ = spdlog::basic_logger_mt("auth_server / Network", "logs/development.log");
  }

  bool Network::init() {
    server_socket_ = engine::network::SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    if (server_socket_ == nullptr) {
      return false;
    }

    SocketAddress server_address(INADDR_ANY, 12345);

    // TODO: Retry when bind() fails
    server_socket_->bind(server_address);

    server_socket_->listen(5);

    mux_ = engine::network::SocketUtil::create_multiplexer();

    if (engine::network::SocketUtil::add_event(mux_, server_socket_) < 0) {
      return false;
    }

    return true;
  }

  int Network::process_incoming_packets() {
    struct kevent events[10];

    // TODO: set timeout
    auto nfds = kevent(mux_, nullptr, 0, events, 10, nullptr);

    if (nfds == -1) {
      return -1;
    } else if (nfds == 0) {
      // timeout
      // ...
      return 0;
    } else {
      std::vector<TCPSocketPtr> ready_sockets;

      for (auto i = 0; i < nfds; i++) {
        auto soc = (int) events[i].ident;

        if (server_socket_->is_same_descriptor(soc)) {
          accept_incoming_packets();
        } else {
          ready_sockets.push_back(client_sockets_.at(soc));
        }
      }

      read_incoming_packets_into_queue(ready_sockets);
    }

    return 0;
  }

  void Network::accept_incoming_packets() {
    auto tcp_socket = server_socket_->accept();
    SocketUtil::add_socket(client_sockets_, tcp_socket);
    SocketUtil::add_event(mux_, tcp_socket);
  }

  void Network::read_incoming_packets_into_queue(const std::vector<TCPSocketPtr> &ready_sockets) {
    char buffer[1500];
    memset(buffer, 0, 1500);

    for (const auto &socket : ready_sockets) {
      auto read_byte_count = socket->recv(buffer, sizeof(buffer));

      if (read_byte_count == 0) {
        // For TCP sockets, the return value 0 means the peer has closed its half side of the connection.
        //HandleConnectionReset( fromAddress );
        auto fd = socket->close();
        client_sockets_.erase(fd);
      } else if(read_byte_count == -engine::network::WSAECONNRESET) {
        //HandleConnectionReset( fromAddress );
        auto fd = socket->close();
        client_sockets_.erase(fd);
      } else if (read_byte_count > 0) {
        buffer[read_byte_count] = '\0';
        std::cout << buffer << std::endl;
      }
      else {
        // uhoh, error? exit or just keep going?
      }
    }
  }
} // namespace auth_server
