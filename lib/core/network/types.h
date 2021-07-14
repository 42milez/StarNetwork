#ifndef STAR_NETWORK_LIB_CORE_NETWORK_TYPES_H_
#define STAR_NETWORK_LIB_CORE_NETWORK_TYPES_H_

#include "lib/rudp/segment.h"

namespace core
{
    struct Payload
    {
      public:
        Payload()
            : segment(nullptr)
            , channel()
            , from(){}

      public:
        std::shared_ptr<rudp::Segment> segment;
        int channel;
        int from;
    };
} // namespace core

#endif // STAR_NETWORK_LIB_CORE_NETWORK_TYPES_H_
