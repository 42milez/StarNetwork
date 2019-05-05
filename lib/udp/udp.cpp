#include <arpa/inet.h>

#include "core/os.h"
#include "core/singleton.h"

#include "peer.h"
#include "udp.h"

static uint32_t time_base;

uint32_t udp_time_get()
{
    return Singleton<OS>::Instance().get_ticks_msec() - time_base;
}

void udp_time_set(uint32_t new_time_base)
{
    time_base = Singleton<OS>::Instance().get_ticks_msec() - new_time_base;
}

void
udp_address_set_ip(const std::unique_ptr<UdpAddress> &address, const uint8_t *ip, size_t size)
{
    auto len = size > 16 ? 16 : size;
    memset(address->host, 0, 16);
    memcpy(address->host, ip, len); // network byte-order (big endian)
}

Error
udp_socket_bind(std::unique_ptr<Socket> &socket, const std::unique_ptr<UdpAddress> &address)
{
    IpAddress ip{};

    if (address->wildcard)
    {
        ip = IpAddress("*");
    }
    else
    {
        ip.set_ipv6(address->host);
    }

    return socket->bind(ip, address->port);
}

UdpAddress::UdpAddress() : port(0), wildcard(0)
{
    memset(&host, 0, sizeof(host));
}

UdpChannel::UdpChannel() : reliable_windows(PEER_RELIABLE_WINDOWS),
                           outgoing_reliable_sequence_number(0),
                           outgoing_unreliable_seaquence_number(0),
                           incoming_reliable_sequence_number(0),
                           incoming_unreliable_sequence_number(0),
                           used_reliable_windows(0)
{}

UdpEvent::UdpEvent() : type(UdpEventType::NONE),
                       channel_id(-1),
                       data(0)
{}
