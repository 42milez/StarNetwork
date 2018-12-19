#include <iostream>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace auth_server {
  namespace {
    using SocketAddress = engine::network::SocketAddress;
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

    auto error = engine::network::SocketUtil::add_event(mux_, server_socket_);

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

    client_sockets_.insert(std::make_pair(tcp_socket->descriptor(), tcp_socket));

    struct kevent event{
      (uintptr_t) tcp_socket->descriptor(),
      EVFILT_READ,
      EV_ADD | EV_CLEAR,
      0,
      0,
      nullptr
    };

    auto error = kevent(mux_, &event, 1, nullptr, 0, nullptr);

    if (error == -1) {
      // error handling
      // ...
    }
  }

  void Network::read_incoming_packets_into_queue(const std::vector<TCPSocketPtr> &ready_sockets) {
    char buffer[1500];

    for (const auto &socket : ready_sockets) {
      auto bytes_received_count = socket->recv(buffer, sizeof(buffer));

      if (bytes_received_count == -1) {
        // handle error
        // ...
      } else if (bytes_received_count == 0) {
        auto fd = socket->close();
        client_sockets_.erase(fd);
      } else {
        buffer[bytes_received_count] = '\0';
        std::cout << buffer << std::endl;
      }
    }
  }
} // namespace auth_server
