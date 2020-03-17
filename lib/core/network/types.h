#ifndef P2P_TECHDEMO_LIB_CORE_NETWORK_TYPES_H_
#define P2P_TECHDEMO_LIB_CORE_NETWORK_TYPES_H_

#include "lib/rudp/segment.h"

namespace core
{
    struct Payload
    {
      public:
        Payload()
            : segment(nullptr)
            , from()
            , channel(){};

      public:
        std::shared_ptr<rudp::Segment> segment;
        int channel;
        int from;
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_NETWORK_TYPES_H_
