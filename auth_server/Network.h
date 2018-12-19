#ifndef LIFE_AUTHSERVER_NETWORK_H
#define LIFE_AUTHSERVER_NETWORK_H

#include <map>

#include <spdlog/spdlog.h>

#include "engine/network/TCPSocket.h"

namespace auth_server
{
  using TCPSocketPtr = engine::network::TCPSocketPtr;

  class Network {
  public:
    Network();

    bool init();

    int process_incoming_packets();

  private:
    void accept_incoming_packets();

    void read_incoming_packets_into_queue(const std::vector<TCPSocketPtr> &ready_sockets);

    std::shared_ptr<spdlog::logger> logger_;

    TCPSocketPtr server_socket_;

    std::map<int, TCPSocketPtr> client_sockets_;

    std::array<int, 1024> queue_;

    int mux_;
  };
} // namespace auth_server

#endif // LIFE_AUTHSERVER_NETWORK_H
