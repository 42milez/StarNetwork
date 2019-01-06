#ifndef P2P_TECHDEMO_AUTHSERVER_NETWORK_H
#define P2P_TECHDEMO_AUTHSERVER_NETWORK_H

#include <map>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <spdlog/spdlog.h>

#include "engine/network/TCPSocket.h"

namespace auth_server
{
  namespace {
    using TCPSocketPtr = engine::network::TCPSocketPtr;
    const int N_KEVENTS = 1024;
  }

  class Network {
  public:
    bool init();

    void process_incoming_packets();

    void store_client(const TCPSocketPtr &tcp_socket);

    void delete_client(int key);

  private:
    int check_kernel_event(struct kevent events[]);

    void accept_incoming_packets();

    void read_incoming_packets_into_queue(const std::vector<TCPSocketPtr> &ready_sockets);

    TCPSocketPtr server_socket_;

    std::map<int, TCPSocketPtr> client_sockets_;

    struct kevent events_[N_KEVENTS];

    int mux_;
  };
} // namespace auth_server

#endif // P2P_TECHDEMO_AUTHSERVER_NETWORK_H
