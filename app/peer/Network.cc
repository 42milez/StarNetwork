#include <cstddef>
#include <future>

#include "Network.h"

namespace peer
{
bool
Network::Init() {
    return true;
}

//std::vector<TCPSocketPtr>
//Network::wait() {
//    std::vector<TCPSocketPtr> in_sockets{ tcp_socket_ };
//    std::vector<TCPSocketPtr> out_sockets;
//
//    engine::network::SocketUtil::wait_for_receiving(kernel_event_queue_fd_, in_sockets, out_sockets);
//
//    return std::move(out_sockets);
//}

} // namespace peer
