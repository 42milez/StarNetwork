#ifndef P2P_TECHDEMO_PLATFORM_UNIX_IO_IP_UNIX_H
#define P2P_TECHDEMO_PLATFORM_UNIX_IO_IP_UNIX_H

#include "core/io/ip.h"
#include "core/io/ip_address.h"

namespace platform { namespace unix { namespace io
{
    class IpUnix : public core::io::IP
    {
    private:
        virtual core::io::IpAddress _resolve_hostname(const std::string &hostname, core::io::IP::Type type);
    };
}}} // namespace platform / unix / io

#endif // P2P_TECHDEMO_PLATFORM_UNIX_IO_IP_UNIX_H
