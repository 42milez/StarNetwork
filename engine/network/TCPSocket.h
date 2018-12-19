#ifndef LIFE_TCPSOCKET_H
#define LIFE_TCPSOCKET_H

#include <memory>

#include "NetworkShared.h"
#include "SocketAddress.h"

namespace engine
{
  namespace network
  {
    class TCPSocket;
    using TCPSocketPtr = std::shared_ptr<TCPSocket>;

    class TCPSocket {
    public:
      ~TCPSocket();

      // int connect();

      int bind(const SocketAddress &address);

      int connect(const SocketAddress &address);

      int listen(int backlog);

      TCPSocketPtr accept();

      ssize_t send(const void *data, size_t len);

      // int32_t receive();

      ssize_t recv(void *buffer, size_t len);

      bool is_same_descriptor(int fd);

      int close();

      int descriptor();

    private:
      friend class SocketUtil;

      explicit TCPSocket(Socket in_socket) : socket_(in_socket) {}

      Socket socket_;
    };
  }
}

#endif // LIFE_TCPSOCKET_H
