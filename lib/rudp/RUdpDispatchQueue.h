#ifndef P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
#define P2P_TECHDEMO_RUDPDISPATCHQUEUE_H

#include <memory>
#include <queue>

#include "RUdpPeer.h"

class RUdpDispatchQueue
{
public:
    void Enqueue(std::shared_ptr<RUdpPeer> &peer);
    std::shared_ptr<RUdpPeer> Dequeue();

public:
    bool PeerExists();

private:
    std::queue<std::shared_ptr<RUdpPeer>> queue_;
};

#endif // P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
