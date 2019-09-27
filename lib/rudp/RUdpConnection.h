#ifndef P2P_TECHDEMO_RUDPCONNECTION_H
#define P2P_TECHDEMO_RUDPCONNECTION_H

#include "core/io/socket.h"

#include "RUdpAddress.h"
#include "RUdpChamber.h"

class RUdpConnection
{
public:
    RUdpConnection(const RUdpAddress &address);
    ssize_t Receive(RUdpAddress &received_address, std::vector<uint8_t> &buffer, size_t buffer_count);
    ssize_t Send(const RUdpAddress &address, const std::unique_ptr<RUdpChamber> &chamber);

private:
    std::unique_ptr<Socket> socket_;
};

#endif // P2P_TECHDEMO_RUDPCONNECTION_H
