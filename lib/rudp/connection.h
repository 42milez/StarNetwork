#ifndef P2P_TECHDEMO_RUDPCONNECTION_H
#define P2P_TECHDEMO_RUDPCONNECTION_H

#include "core/io/socket.h"

#include "network_config.h"
#include "chamber.h"

namespace rudp
{
    class Connection
    {
    public:
        explicit
        Connection(const NetworkConfig &address);

        ssize_t
        Receive(NetworkConfig &received_address, VecUInt8 &buffer, size_t buffer_count);

        ssize_t
        Send(const NetworkConfig &address, const std::unique_ptr<Chamber> &chamber);

    private:
        std::unique_ptr<Socket> socket_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCONNECTION_H