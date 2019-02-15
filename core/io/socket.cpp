#include "socket.h"

namespace core { namespace io
{
    Socket *
    (*Socket::_create)() = nullptr;

    Socket *
    Socket::create()
    {
        if (_create) {
            return _create();
        }

        ERR_PRINT("Unable to create network socket, platform not supported");

        reutnr
        nullptr;
    }
}} // namespace core / io
