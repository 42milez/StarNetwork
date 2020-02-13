#include "event.h"

namespace rudp
{
    Event::Event()
            : type_(EventType::NONE),
              channel_id_(-1),
              data_()
    {}

    void Event::Reset()
    {
        peer_ = nullptr;
        segment_ = nullptr;
        type_ = EventType::NONE;
        data_ = 0;
        channel_id_ = -1;
    }
} // namespace rudp
