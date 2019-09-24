#include "RUdpAddress.h"

#include <cstring>

RUdpAddress::RUdpAddress()
    : host_(),
      port_(),
      wildcard_()
{}

void
RUdpAddress::Reset()
{
    std::fill_n(host_.begin(), HOST_LENGTH, 0);
    port_ = 0;
    wildcard_ = 0;
}

void
RUdpAddress::SetIP(const uint8_t *ip, size_t size)
{
    auto len = size > HOST_LENGTH ? HOST_LENGTH : size;

    std::fill_n(host_.begin(), HOST_LENGTH, 0);

    memcpy(&host_, ip, len); // network byte-order (big endian)
}

RUdpAddress &
RUdpAddress::operator=(const RUdpAddress &address)
{
    if (this == &address)
        return *this;

    std::copy(address.host().begin(), address.host().end(), host_.begin());
    port_ = address.port();

    return *this;
}

bool
RUdpAddress::operator==(const RUdpAddress &address) const
{
    auto same_host = memcmp(&host_, &address.host(), HOST_LENGTH) == 0;
    auto same_port = port_ == address.port();

    return same_host & same_port;
}

bool
RUdpAddress::operator!=(const RUdpAddress &address) const
{
    auto same_host = memcmp(&host_, &address.host(), HOST_LENGTH) == 0;
    auto same_port = port_ == address.port();

    return !(same_host & same_port);
}
