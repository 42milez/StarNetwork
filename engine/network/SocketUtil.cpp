#include <array>
#include <vector>

#include <sys/event.h>
#include <sys/socket.h>

#include "SocketUtil.h"

namespace network
{
  namespace
  {
    bool is_socket_ready(int socket, struct kevent events[], int nfds) {
      for (auto i = 0; i < nfds; i++) {
        if (events[i].ident == socket) return true;
      }
      return false;
    }
  }

  std::shared_ptr<spdlog::logger> SocketUtil::logger_ = spdlog::stdout_color_mt("SocketUtil");

  bool SocketUtil::static_init() {
    auto mux_ = ::kqueue();

    if (mux_ == -1) {
      return -1;
    }

    return true;
  }

  TCPSocketPtr SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily in_family) {
    int socket = ::socket(static_cast<int>(in_family), SOCK_STREAM, IPPROTO_TCP);

    if (socket != engine::network::INVALID_SOCKET) {
      return TCPSocketPtr(new TCPSocket(socket));
    } else {
      report_error("SocketUtil::create_tcp_socket");
      return nullptr;
    }
  }

  void SocketUtil::add_event(const TCPSocketPtr socket) {
    struct kevent event{
      (uintptr_t) socket->socket_,
      EVFILT_READ,
      EV_ADD | EV_DELETE | EV_CLEAR,
      0,
      0,
      nullptr
    };
    kevent(mux_, &event, 1, nullptr, 0, nullptr);
  }

  int SocketUtil::wait(std::vector<TCPSocketPtr> &in_sockets, std::vector<TCPSocketPtr> &out_sockets) {
    // TODO Consider the size of events. The size may affect the read throughput from descriptor.
    const auto n_events = 10;
    struct kevent events[n_events];
    auto nfds = kevent(mux_, nullptr, 0, events, n_events, nullptr);

    if (nfds == -1) {
      return -1;
    } else if (nfds == 0) {
      return 0;
    } else {
      for (const auto &socket : in_sockets) {
        if (is_socket_ready(socket->socket_, events, nfds)) {
          out_sockets.push_back(socket);
        }
      }
    }

    return nfds;
  }

  void SocketUtil::report_error(const char *in_operation_desc) {
    logger_->error("Error: {0}", in_operation_desc);
  }

  int SocketUtil::last_error() {
    return errno;
  }

} // namespace network
