#ifndef LIFE_TCPSOCKET_H
#define LIFE_TCPSOCKET_H

#include <memory>

#include "NetworkShared.h"
#include "SocketAddress.h"

namespace network {

  class TCPSocket {
  public:
    ~TCPSocket();

    // int connect();

    int bind(const SocketAddress& address);

    int connect(const SocketAddress& address);

    int listen();

    // std::shared_ptr<TCPSocket> accept();

    int32_t send(const void* data, size_t len);

    // int32_t receive();

    void recv();

  private:
    friend class SocketUtil;

    explicit TCPSocket(Socket in_socket) : socket_(in_socket) {}

    Socket socket_;
  };

  using TCPSocketPtr = std::shared_ptr<TCPSocket>;

}

#endif // LIFE_TCPSOCKET_H
