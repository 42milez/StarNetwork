#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "ip_unix.h"

#include "platform/unix/io/ip_unix.h"

#define INIT_ADDRINFO_AS_HINTS() \
    { 0, 0, 0, 0, 0, nullptr, nullptr, nullptr }

enum class AddrinfoRetVal : int
{
    SUCCESS = 0
};

namespace platform { namespace unix { namespace io
{
    static IpAddress
    _sockaddr2ip(struct sockaddr &addr)
    {
        IpAddress ip;

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

    IpAddress
    IpUnix::_resolve_hostname(const std::string &hostname, IP::Type type)
    {
        struct addrinfo hints = INIT_ADDRINFO_AS_HINTS();
        struct addrinfo *results;

        if (type == IP::Type::V4)
        {
            hints.ai_family = AF_INET;
        }
        else if (type == IP::Type::V6)
        {
            hints.ai_family = AF_INET6;
            hints.ai_flags = 0;
        }
        else
        {
            hints.ai_family = AF_UNSPEC;
            hints.ai_flags = AI_ADDRCONFIG;
        }

        hints.ai_flags &= ~AI_NUMERICHOST;

        auto s = getaddrinfo(hostname.c_str(), nullptr, &hints, &results);

        if (static_cast<AddrinfoRetVal>(s) != AddrinfoRetVal::SUCCESS)
        {
            // ToDo: logging
            // ...

            return IpAddress();
        }

        if (results == nullptr || results->ai_addr == nullptr)
        {
            // ToDo: logging
            // ...

            if (results)
            {
                freeaddrinfo(results);
            }

            return IpAddress();
        }

        IpAddress ip = _sockaddr2ip(*(results->ai_addr));

        freeaddrinfo(results);

        return ip;
    }
}}} // namespace platform / unix / io
