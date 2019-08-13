#include <core/error_macros.h>

#include "RUdpChamber.h"
#include "RUdpConnection.h"

RUdpConnection::RUdpConnection(const RUdpAddress &address)
{
    _socket = std::make_unique<Socket>();

    if (_socket == nullptr) {
        // throw exception
        // ...
    }

    _socket->open(Socket::Type::UDP, IP::Type::ANY);

    _socket->set_blocking_enabled(false);
    _socket->set_broadcasting_enabled(true);

    IpAddress ip{};

    if (address.wildcard)
        ip = IpAddress("*");
    else
        ip.set_ipv6(address.host);

    auto ret = _socket->bind(ip, address.port);

    if (ret != Error::OK) {
        // throw exception
        // ...
    }
}

ssize_t
RUdpConnection::receive(RUdpAddress &received_address, std::vector<uint8_t> &buffer, size_t buffer_count)
{
    ERR_FAIL_COND_V(buffer_count != 1, -1)

    Error err = _socket->poll(Socket::PollType::POLL_TYPE_IN, 0);

    if (err == Error::ERR_BUSY)
        return 0;

    if (err != Error::OK)
        return -1;

    ssize_t read_count;
    IpAddress ip;

    err = _socket->recvfrom(buffer, read_count, ip, received_address.port);

    if (err == Error::ERR_BUSY)
        return 0;

    if (err != Error::OK)
        return -1;

    received_address.SetIP(ip.GetIPv6(), 16);

    return read_count;
}

ssize_t
RUdpConnection::send(const RUdpAddress &address, const std::unique_ptr<RUdpChamber> &chamber)
{
    IpAddress dest;

    dest.set_ipv6(address.host);

    std::vector<uint8_t> out;

    auto size = chamber->Write(out);

    ssize_t sent = 0;

    auto debug_header = reinterpret_cast<RUdpProtocolHeader *>(&(out.at(0)));
    auto debug_command = reinterpret_cast<RUdpProtocolType *>(&(out.at(8)));

    auto err = _socket->sendto(&(out.at(0)), size, sent, dest, address.port);

    if (err != Error::OK) {
        if (err == Error::ERR_BUSY)
            return 0;

        return -1;
    }

    return sent;
}
