#include <vector>

#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "SocketUtil.h"
#include "TCPSocket.h"

namespace engine
{
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

    int SocketUtil::create_multiplexer() {
      auto mux = kqueue();

      if (mux == -1) {
        return -1;
      }

      return mux;
    }

    void SocketUtil::add_socket(std::map<int, TCPSocketPtr> &sockets, const TCPSocketPtr &socket) {
      sockets.insert(std::make_pair(socket->socket_, socket));
    }

    KEVENT_REGISTER_STATUS SocketUtil::register_event(int mux, const TCPSocketPtr &socket) {
      struct kevent event{
        (uintptr_t) socket->socket_,
        EVFILT_READ,
        EV_ADD | EV_CLEAR,
        0,
        0,
        nullptr
      };
      return static_cast<KEVENT_REGISTER_STATUS>(kevent(mux, &event, 1, nullptr, 0, nullptr));
    }

    int SocketUtil::wait_for_receiving(int mux, const std::vector<TCPSocketPtr> &in_sockets, std::vector<TCPSocketPtr> &out_sockets) {
      // TODO Consider the size of events. The size may affect the read throughput from descriptor.
      struct kevent events[10];

      auto nfds = kevent(mux, nullptr, 0, events, 10, nullptr);

      if (nfds == -1) {
        return -1;
      } else if (nfds == 0) {
        // timeout
        // ...
        return 0;
      } else {
        for (auto i = 0; i < nfds; i++) {
          auto soc = (int) events[i].ident;
          for (const auto &socket : in_sockets) {
            if (soc == socket->socket_) {
              out_sockets.push_back(TCPSocketPtr(new TCPSocket(soc)));
            }
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
} // namespace engine
