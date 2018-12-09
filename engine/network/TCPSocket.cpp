#include <unistd.h>

#include <sys/event.h>
#include <sys/socket.h>

#include "SocketUtil.h"

namespace network
{

  TCPSocket::~TCPSocket() {
    close(socket_);
  }

  int TCPSocket::bind(const SocketAddress &address) {
    int error = ::bind(socket_, &address.sockaddr_, address.size());
    if (error != 0) {
      SocketUtil::report_error("TCPSocket::bind");
      return SocketUtil::last_error();
    }

    return NO_ERROR;
  }

  int TCPSocket::connect(const SocketAddress &address) {
    int err = ::connect(socket_, &address.sockaddr_, address.size());
    if (err < 0) {
      SocketUtil::report_error("TCPSocket::Connect");
      return SocketUtil::last_error();
    }
    return NO_ERROR;
  }

  int32_t TCPSocket::send(const void *data, size_t len) {
    auto bytes_sent = ::send(socket_, static_cast<const char *>(data), len, 0);
    if (bytes_sent < 0) {
      SocketUtil::report_error("TCPSocket::send");
      return SocketUtil::last_error();
    }
    // TODO using suitable data type for platform
    return bytes_sent;
  }

  void TCPSocket::recv() {
    auto mux = kqueue();
    if (mux == -1) {
      return;
    }
    struct kevent event{(uintptr_t) socket_, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr};
    kevent(mux, &event, 1, nullptr, 0, nullptr);

    struct kevent events[10];
    for (;;) {
      auto nfds = kevent(mux, nullptr, 0, events, 10, nullptr);
      if (nfds == -1) {
        return;
      } else if (nfds == 0) {
        continue;
      } else {
        char *ptr;
        for (auto i = 0; i < nfds; i++) {
          auto soc = (int) events[i].ident;
          if (soc == socket_) {
            std::string buf(512, '\0');
            auto len = recv(soc, &buf[0], sizeof(buf), 0);
            buf.resize(len);
          }
        }
      }
    }
  }
}
