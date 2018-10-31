#ifndef LIFE_TCPSOCKET_H
#define LIFE_TCPSOCKET_H

#include <memory>

#include <spdlog/spdlog.h>

namespace network
{
  class TcpSocket {
  public:
    TcpSocket();

    ~TcpSocket();

    int fd();

  private:
    int create_socket();

    std::shared_ptr<spdlog::logger> logger_;

    int fd_;
  };
} // namespace network

#endif // LIFE_TCPSOCKET_H
