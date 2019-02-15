#include <sys/socket.h>
#include <netinet/in.h>

#include "ip_unix.h"

#include "platform/unix/io/ip_unix.h"

namespace platform { namespace unix { namespace io
{
    static core::io::IpAddress
    _sockaddr2ip(struct sockaddr &addr)
    {
        core::io::IpAddress ip;

        if (addr.sa_family == AF_INET)
        {
            auto &addr_in = reinterpret_cast<struct sockaddr_in &>(addr);

            auto octet1 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 24);
            auto octet2 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 16);
            auto octet3 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 8);
            auto octet4 = static_cast<uint8_t>(addr_in.sin_addr.s_addr);

            ip.set_ipv4({octet1, octet2, octet3, octet4});
        }
        else if (addr.sa_family == AF_INET6)
        {
            auto &addr_in6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

            ip.set_ipv6(addr_in6.sin6_addr.s6_addr);
        }

        return ip;
    }
}}} // namespace platform / unix / io
