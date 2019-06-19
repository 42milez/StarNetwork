#ifndef P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H
#define P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H

#include "RUdpCommon.h"

using RUdpAcknowledgement = struct RUdpAcknowledgement
{
    uint32_t sent_time;
    RUdpProtocolType command;
};

#endif // P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H
