#include "lib/core/error_macros.h"
#include "lib/core/logger.h"
#include "lib/core/singleton.h"

#include "chamber.h"
#include "connection.h"

namespace rudp
{
    Connection::Connection(const NetworkConfig &address)
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

        if (address.wildcard()) {
            ip = IpAddress("*");
        }
        else {
            ip.set_ipv6(address.host());
        }

        auto ret = socket_->bind(ip, address.port());

        if (ret != Error::OK) {
            // throw exception
            // ...
        }
    }

    ssize_t
    Connection::Receive(NetworkConfig &received_address, std::vector<uint8_t> &buffer, size_t buffer_count)
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
        err           = socket_->recvfrom(buffer, read_count, ip, port);
        received_address.port(port);

        if (err == Error::ERR_BUSY)
            return 0;

        if (err != Error::OK)
            return -1;

        received_address.SetIP(ip.GetIPv6(), 16);

        LOG_DEBUG_VA("received length: {0}", read_count)

        return read_count;
    }

    ssize_t
    Connection::Send(const NetworkConfig &address, const std::unique_ptr<Chamber> &chamber)
    {
        IpAddress dest;

        dest.set_ipv6(address.host());

        std::vector<uint8_t> out;

        auto size = chamber->Write(out);

        LOG_DEBUG_VA("data wrote: {0}", std::string{out.begin(), out.end()})

        ssize_t sent = 0;

        auto err = socket_->sendto(&(out.at(0)), size, sent, dest, address.port());

        LOG_DEBUG_VA("bytes sent: {0}", sent)

        if (err != Error::OK) {
            if (err == Error::ERR_BUSY)
                return 0;

            return -1;
        }

        return sent;
    }
} // namespace rudp
