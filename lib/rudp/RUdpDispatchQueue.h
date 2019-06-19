#ifndef P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
#define P2P_TECHDEMO_RUDPDISPATCHQUEUE_H

#include <memory>
#include <queue>

#include "RUdpPeer.h"

class UdpDispatchQueue
{
private:
    std::queue<std::shared_ptr<RUdpPeer>> _queue;

public:
    std::shared_ptr<RUdpPeer> pop_peer();

    void push(std::shared_ptr<RUdpPeer> &peer);

    bool peer_exists();
};

#endif // P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
