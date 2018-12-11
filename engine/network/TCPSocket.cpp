#include <unistd.h>

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

  int32_t TCPSocket::recv(void *data, size_t len) {
    int bytesReceivedCount = ::recv(socket_, static_cast<char *>(data), len, 0);
    if (bytesReceivedCount < 0) {
      SocketUtil::report_error("TCPSocket::Receive");
      return -SocketUtil::last_error();
    }
    return bytesReceivedCount;
  }
}
