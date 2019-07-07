#ifndef P2P_TECHDEMO_RUDPCONNECTION_H
#define P2P_TECHDEMO_RUDPCONNECTION_H

#include "core/io/socket.h"

#include "RUdpAddress.h"
#include "RUdpChamber.h"

class RUdpConnection
{
private:
    std::unique_ptr<Socket> _socket;

public:
    RUdpConnection(const RUdpAddress &address);

    ssize_t receive(RUdpAddress &received_address, std::vector<uint8_t> &buffer, size_t buffer_count);

    ssize_t send(const RUdpAddress &address, const std::unique_ptr<RUdpChamber> &chamber);
};

#endif // P2P_TECHDEMO_RUDPCONNECTION_H
