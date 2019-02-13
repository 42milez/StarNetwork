#ifndef P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H
#define P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H

#include <sys/socket.h>

#include "core/io/socket.h"

using SOCKET_TYPE = int;

namespace platform { namespace unix { namespace io
{
    class SocketUnix : public core::io::Socket
    {
    private:
        SOCKET_TYPE _sock;

    };
}}} // namespace platform / unix / io

#endif // P2P_TECHDEMO_PLATFORM_UNIX_IO_SOCKETUNIX_H
