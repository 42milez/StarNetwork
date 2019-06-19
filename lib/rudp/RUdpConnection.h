#ifndef P2P_TECHDEMO_RUDPCONNECTION_H
#define P2P_TECHDEMO_RUDPCONNECTION_H

#include "core/io/socket.h"

#include "RUdpAddress.h"

class RUdpConnection
{
private:
    std::unique_ptr<Socket> _socket;

public:
    RUdpConnection(const UdpAddress &address);

    ssize_t send(const UdpAddress &address, const std::shared_ptr<RUdpChamber> &chamber);
};

#endif // P2P_TECHDEMO_RUDPCONNECTION_H
