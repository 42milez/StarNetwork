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

    ssize_t receive(std::unique_ptr<RUdpAddress> &received_address, RUdpBuffer &buffer, size_t buffer_count);

    ssize_t send(const RUdpAddress &address, const std::unique_ptr<RUdpChamber> &chamber);
};

#endif // P2P_TECHDEMO_RUDPCONNECTION_H
