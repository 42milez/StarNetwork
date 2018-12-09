#include <sys/socket.h>

#include "SocketUtil.h"

namespace network {

  std::shared_ptr<spdlog::logger> SocketUtil::logger_ = spdlog::stdout_color_mt("SocketUtil");

  TCPSocketPtr SocketUtil::create_tcp_socket(SocketAddressFamily in_family) {
    Socket s = socket(static_cast<int>(in_family), SOCK_STREAM, IPPROTO_TCP);

    if (s != INVALID_SOCKET) {
      return TCPSocketPtr(new TCPSocket(s));
    } else {
      report_error("SocketUtil::create_tcp_socket");
      return nullptr;
    }
  }

  void SocketUtil::report_error(const char *in_operation_desc) {
    logger_->error("Error: {0}", in_operation_desc);
  }

  int SocketUtil::last_error() {
    return errno;
  }

} // namespace network
