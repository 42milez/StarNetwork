#include <arpa/inet.h>
#include <linux/if.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "engine/util/internal_error_exception.h"
#include "tcp_socket.h"

namespace network
{
  TcpSocket::TcpSocket() {
    logger_ = spdlog::stdout_color_mt("TcpSocket");
    fd_ = create_socket();
  }

  TcpSocket::~TcpSocket() {
    close(fd_);
  }

  int TcpSocket::create_socket(int port) {
    struct addrinfo hints{AI_PASSIVE, AF_INET, SOCK_STREAM, 0, 0, nullptr, nullptr, nullptr};
    struct addrinfo *res0;
    int err;

    if ((err = getaddrinfo(nullptr, nullptr, &hints, &res0)) != 0) {
      logger_->critical("getaddrinfo(): {0}", gai_strerror(err));
      throw util::InternalErrorException{"Cannot create socket."};
    }

    char nbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];

    if ((err = getnameinfo(res0->ai_addr,
                           res0->ai_addrlen,
                           nbuf,
                           sizeof(nbuf),
                           sbuf,
                           sizeof(sbuf),
                           NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
      freeaddrinfo(res0);
      logger_->critical("getnameinfo(): {0}", gai_strerror(err));
      throw util::InternalErrorException{"Cannot create socket."};
    }

    int fd;

    if ((fd = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol)) == -1) {
      logger_->critical("socket()");
      freeaddrinfo(res0);
      throw util::InternalErrorException{"Cannot create socket."};
    }

    int opt{1};
    socklen_t opt_len{sizeof(opt)};

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, opt_len) == -1) {
      logger_->critical("setsockopt()");
      close(fd);
      freeaddrinfo(res0);
      throw util::InternalErrorException{"Cannot create socket."};
    }

    if (bind(fd, res0->ai_addr, res0->ai_addrlen) == -1) {
      logger_->critical("bind()");
      close(fd);
      freeaddrinfo(res0);
      throw util::InternalErrorException{"Cannot create socket."};
    }

    if (listen(fd, SOMAXCONN) == -1) {
      logger_->critical("listen()");
      close(fd);
      freeaddrinfo(res0);
      throw util::InternalErrorException{"Cannot create socket."};
    }

    freeaddrinfo(res0);

    return fd;
  }

  int TcpSocket::fd() {
    return fd_;
  }
} // namespace network
