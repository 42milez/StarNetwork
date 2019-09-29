#ifndef P2P_TECHDEMO_NETWORK_H
#define P2P_TECHDEMO_NETWORK_H

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

namespace peer
{
  class Network {
  public:
    bool Init();
    //std::vector<TCPSocketPtr> wait();

//  private:
//    engine::network::TCPSocketPtr tcp_socket_;
//
//    int kernel_event_queue_fd_;
  };

} // namespace peer

#endif // P2P_TECHDEMO_NETWORK_H
