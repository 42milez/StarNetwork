#include <unistd.h>

#include <sys/socket.h>

#include "core/RUdpLogger.h"
#include "core/singleton.h"

#include "SocketUtil.h"

namespace engine
{
  namespace network
  {
    namespace
    {
      using Logger = core::Singleton<core::Logger>;
      using SocketUtil = engine::network::SocketUtil;
    }

    TCPSocket::~TCPSocket() {
      close();
    }

    SOCKET_STATUS TCPSocket::bind(const SocketAddress &address) {
      return static_cast<SOCKET_STATUS>(::bind(socket_, &address.sockaddr_, address.size()));
    }

    int TCPSocket::connect(const SocketAddress &address) {
      int err = ::connect(socket_, &address.sockaddr_, address.size());
      if (err < 0) {
        return SocketUtil::last_error();
      }
      return NO_ERROR;
    }

    ssize_t TCPSocket::send(const void *data, size_t len) {
      return ::send(socket_, static_cast<const char *>(data), len, 0);
    }

    ssize_t TCPSocket::recv(void *data, size_t len) {
      ssize_t bytes_received_count = ::recv(socket_, static_cast<char *>(data), len, 0);
      if (bytes_received_count < 0) {
        return SocketUtil::last_error();
      }
      return bytes_received_count;
    }

    SOCKET_STATUS TCPSocket::listen(int backlog) {
      return static_cast<SOCKET_STATUS>(::listen(socket_, backlog));
    }

    TCPSocketPtr TCPSocket::accept() {
      struct sockaddr_storage addr{};
      socklen_t length = sizeof(addr);
      int new_socket = ::accept(socket_, (struct sockaddr *) &addr, &length );

      if (new_socket != INVALID_SOCKET ) {
        return TCPSocketPtr(new TCPSocket(new_socket));
      }
      else {
        return nullptr;
      }
    }

    bool TCPSocket::is_same_descriptor(int fd) {
      return fd == socket_;
    }

    int TCPSocket::close() {
      ::close(socket_);
      return socket_;
    }

    int TCPSocket::descriptor() {
      return socket_;
    }
  } // namespace network
} // namespace engine
