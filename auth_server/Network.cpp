#include <iostream>

#include "engine/base/Singleton.h"
#include "engine/base/Logger.h"
#include "engine/network/NetworkShared.h"
#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace auth_server
{
  namespace
  {
    using Logger = engine::base::Singleton<engine::base::Logger>;
    using SocketAddress = engine::network::SocketAddress;
    using SocketAddressFamily = engine::network::SocketAddressFamily;
    using SocketUtil = engine::network::SocketUtil;

    using KEVENT_STATUS = engine::network::KEVENT_STATUS;
    using KQUEUE_STATUS = engine::network::KQUEUE_STATUS;
    using SOCKET_STATUS = engine::network::SOCKET_STATUS;

    const int LISTEN_BACKLOG = 5;
    const uint32_t SERVER_ADDRESS = INADDR_ANY;
    const uint16_t SERVER_PORT = 12345;

    const int READ_BUFFER_SIZE = 1500;
  }

  bool Network::init() {
    server_socket_ = SocketUtil::create_tcp_socket(SocketAddressFamily::INET);

    if (server_socket_ == nullptr) {
      Logger::Instance().critical("Cannot create server socket.");
      return false;
    }

    SocketAddress server_address(SERVER_ADDRESS, SERVER_PORT);

    if (server_socket_->bind(server_address) == SOCKET_STATUS::FAIL) {
      Logger::Instance().critical("bind() failed [{0}]", SocketUtil::last_error());
      return false;
    }

    if (server_socket_->listen(LISTEN_BACKLOG) == SOCKET_STATUS::FAIL) {
      Logger::Instance().critical("listen() failed [{0}]", SocketUtil::last_error());
      return false;
    }

    kernel_event_queue_fd_ = SocketUtil::create_event_interface();

    if (static_cast<KQUEUE_STATUS>(kernel_event_queue_fd_) == KQUEUE_STATUS::FAIL) {
      Logger::Instance().critical("create_event_interface() failed [{0}]", SocketUtil::last_error());
      return false;
    }

    if (SocketUtil::register_event(kernel_event_queue_fd_, server_socket_) == KEVENT_STATUS::FAIL) {
      Logger::Instance().critical("register_event() failed [{0}]", SocketUtil::last_error());
      return false;
    }

    return true;
  }

  void Network::process_incoming_packets() {
    // MEMO: Don't initialize events_ as the nfds indicates the number of the events.
    auto nfds = retrieve_kernel_event(events_);

    if (static_cast<KEVENT_STATUS>(nfds) == KEVENT_STATUS::FAIL) {
      // error
      // TODO: Output logs
      return;
    } else if (static_cast<KEVENT_STATUS>(nfds) == KEVENT_STATUS::TIMEOUT) {
      // timeout
      // TODO: Output logs
      return;
    } else {
      // TODO: Use queue
      std::vector<TCPSocketPtr> ready_sockets;

      for (auto i = 0; i < nfds; i++) {
        auto fd = static_cast<int>(events_[i].ident);

        if (server_socket_->is_same_descriptor(fd)) {
          if (!accept_incoming_packets()) {
            Logger::Instance().error("accept_incoming_packets() failed [{0}]", SocketUtil::last_error());
          }
        } else {
          ready_sockets.push_back(client_sockets_.at(fd));
        }
      }

      read_incoming_packets_into_queue(ready_sockets);
    }
  }

  int Network::retrieve_kernel_event(struct kevent events[]) {
    /*
     KQUEUE(2) / BSD System Calls Manual / KQUEUE(2)

     The kevent(), kevent64() and kevent_qos() system calls return the number of events placed in the eventlist, up to
     the value given by nevents.  If an error occurs while processing an element of the changelist and there is enough
     room in the eventlist, then the event will be placed in the eventlist with EV_ERROR set in flags and the system
     error in data.  Otherwise, -1 will be returned, and errno will be set to indicate the error condition.  If the
     time limit expires, then kevent(), kevent64() and kevent_qos() return 0.
     */
    return kevent(kernel_event_queue_fd_, nullptr, 0, events, N_KEVENTS, nullptr);
  }

  bool Network::accept_incoming_packets() {
    auto tcp_socket = server_socket_->accept();

    if (tcp_socket == nullptr) {
      return false;
    }

    store_client(tcp_socket);

    if (SocketUtil::register_event(kernel_event_queue_fd_, tcp_socket) == KEVENT_STATUS::FAIL) {
      return false;
    }

    return true;
  }

  void Network::read_incoming_packets_into_queue(const std::vector<TCPSocketPtr> &ready_sockets) {
    char buffer[READ_BUFFER_SIZE];
    memset(buffer, 0, READ_BUFFER_SIZE);

    for (const auto &socket : ready_sockets) {
      auto read_byte_count = socket->recv(buffer, READ_BUFFER_SIZE);

      if (SocketUtil::is_connection_reset_on_recv(read_byte_count)) {
        // Memo: For TCP sockets, the return value 0 means the peer has closed its half side of the connection.

        // TODO: Handle connection reset
        // ...

        delete_client(socket->descriptor());
      } else if (SocketUtil::is_no_messages_to_read(read_byte_count)) {
        // Memo: no messages are available.
      } else if (read_byte_count > 0) {
        buffer[read_byte_count] = '\0';
        std::cout << buffer << std::endl;
      } else {
        // uhoh, error? exit or just keep going?
      }
    }
  }

  void Network::store_client(const TCPSocketPtr &tcp_socket) {
    client_sockets_.insert(std::make_pair(tcp_socket->descriptor(), tcp_socket));
  }

  void Network::delete_client(int key) {
    client_sockets_.erase(key);
  }
} // namespace auth_server
