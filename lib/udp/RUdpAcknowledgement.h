#ifndef P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H
#define P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H

#include "RUdpCommon.h"

using UdpAcknowledgement = struct UdpAcknowledgement
{
    uint32_t sent_time;
    UdpProtocolType command;
};

#endif // P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H
