#ifndef P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H
#define P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H

#include "RUdpCommon.h"

class RUdpAcknowledgement
{
public:
    RUdpAcknowledgement();

public:
    inline RUdpProtocolType & command()
    { return command_; }

    inline void command(RUdpProtocolType val)
    { command_ = val; }

    inline uint32_t sent_time()
    { return sent_time_; }

    inline void sent_time(uint32_t val)
    { sent_time_ = val; }

private:
    RUdpProtocolType command_;
    uint32_t sent_time_;
};

#endif // P2P_TECHDEMO_RUDPACKNOWLEDGEMENT_H
