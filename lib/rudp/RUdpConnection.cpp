#include "core/error_macros.h"
#include "core/logger.h"
#include "core/singleton.h"

#include "RUdpChamber.h"
#include "RUdpConnection.h"

RUdpConnection::RUdpConnection(const RUdpAddress &address)
{
    socket_ = std::make_unique<Socket>();

    if (socket_ == nullptr) {
        // throw exception
        // ...
    }

    socket_->open(Socket::Type::UDP, IP::Type::ANY);

    socket_->set_blocking_enabled(false);
    socket_->set_broadcasting_enabled(true);

    IpAddress ip{};

    if (address.wildcard())
    {
        ip = IpAddress("*");
    }
    else
    {
        ip.set_ipv6(address.host());
    }

    auto ret = socket_->bind(ip, address.port());

    if (ret != Error::OK) {
        // throw exception
        // ...
    }
}

ssize_t
RUdpConnection::Receive(RUdpAddress &received_address, VecUInt8 &buffer, size_t buffer_count)
{
    ERR_FAIL_COND_V(buffer_count != 1, -1)

    Error err = socket_->poll(Socket::PollType::POLL_TYPE_IN, 0);

    if (err == Error::ERR_BUSY)
        return 0;

    if (err != Error::OK)
        return -1;

    ssize_t read_count;
    IpAddress ip;

    uint16_t port = 0;
    err = socket_->recvfrom(buffer, read_count, ip, port);
    received_address.port(port);

    if (err == Error::ERR_BUSY)
        return 0;

    if (err != Error::OK)
        return -1;

    received_address.SetIP(ip.GetIPv6(), 16);

    core::Singleton<core::Logger>::Instance().Debug("received length: {0}", read_count);

    return read_count;
}

ssize_t
RUdpConnection::Send(const RUdpAddress &address, const std::unique_ptr<RUdpChamber> &chamber)
{
    IpAddress dest;

    dest.set_ipv6(address.host());

    VecUInt8 out;

    auto size = chamber->Write(out);

    ssize_t sent = 0;

    auto err = socket_->sendto(&(out.at(0)), size, sent, dest, address.port());

    if (err != Error::OK) {
        if (err == Error::ERR_BUSY)
            return 0;

        return -1;
    }

    return sent;
}
