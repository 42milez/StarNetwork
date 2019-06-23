#include "RUdpChamber.h"
#include "RUdpConnection.h"

RUdpConnection::RUdpConnection(const RUdpAddress &address)
{
    _socket = std::make_unique<Socket>();
    _socket->open(Socket::Type::UDP, IP::Type::ANY);
    _socket->set_blocking_enabled(false);

    if (_socket == nullptr) {
        // throw exception
        // ...
    }

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
RUdpConnection::send(const RUdpAddress &address, const std::unique_ptr<RUdpChamber> &chamber)
{
    IpAddress dest;

    dest.set_ipv6(address.host);

    std::vector<uint8_t> out;

    auto size = chamber->write(out);

    ssize_t sent = 0;

    auto err = _socket->sendto(out, size, sent, dest, address.port);

    if (err != Error::OK) {
        if (err == Error::ERR_BUSY)
            return 0;

        return -1;
    }

    return sent;
}
