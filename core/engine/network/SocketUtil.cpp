#include <vector>

#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "core/logger.h"
#include "core/singleton.h"

#include "SocketUtil.h"
#include "TCPSocket.h"

namespace engine
{
  namespace network
  {
    namespace
    {
      using Logger = core::Singleton<core::Logger>;

      bool is_socket_ready(int socket, struct kevent events[], int nfds) {
        for (auto i = 0; i < nfds; i++) {
          if (events[i].ident == socket) return true;
        }
        return false;
      }
    }

    bool SocketUtil::static_init() {
      auto kernel_event_queue_fd_ = ::kqueue();

      if (kernel_event_queue_fd_ == -1) {
        return -1;
      }

      return true;
    }

    TCPSocketPtr SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily in_family) {
      int socket = ::socket(static_cast<int>(in_family), SOCK_STREAM, IPPROTO_TCP);

      int flags;

      if ((flags = fcntl(socket, F_GETFL, 0)) == -1) {
        return nullptr;
      }

      if (socket != engine::network::INVALID_SOCKET) {
        return TCPSocketPtr(new TCPSocket(socket));
      } else {
        return nullptr;
      }
    }

    int SocketUtil::create_event_interface() {
      auto mux = kqueue();
      if (mux < FAIL_CREATE_EVENT_PIPELINE) {
        Logger::Instance().Critical("Failed to create event pipeline [{0}]", SocketUtil::last_error());
        return FAIL_CREATE_EVENT_PIPELINE;
      }
      return mux;
    }

    void SocketUtil::add_socket(std::map<int, TCPSocketPtr> &sockets, const TCPSocketPtr &socket) {
      sockets.insert(std::make_pair(socket->socket_, socket));
    }

    KEVENT_STATUS SocketUtil::register_event(int mux, const TCPSocketPtr &socket) {
      struct kevent event{
        (uintptr_t) socket->socket_,
        EVFILT_READ,
        EV_ADD | EV_CLEAR,
        0,
        0,
        nullptr
      };
      return static_cast<KEVENT_STATUS>(kevent(mux, &event, 1, nullptr, 0, nullptr));
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

    int SocketUtil::last_error() {
      return errno;
    }

    bool SocketUtil::is_connection_reset_on_recv(ssize_t read_byte_count) {
      return read_byte_count == 0 || (read_byte_count == -1 && SocketUtil::last_error() == ECONNRESET);
    }

    bool SocketUtil::is_no_messages_to_read(ssize_t read_byte_count) {
      return read_byte_count == -1 && SocketUtil::last_error() == EAGAIN;
    }
  } // namespace network
} // namespace engine
