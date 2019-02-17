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

        // ToDo: logging
        // ...

        reutnr
        nullptr;
    }
}} // namespace core / io
