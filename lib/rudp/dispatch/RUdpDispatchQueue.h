#ifndef P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
#define P2P_TECHDEMO_RUDPDISPATCHQUEUE_H

#include <memory>
#include <queue>

#include "lib/rudp/peer/RUdpPeer.h"

namespace rudp
{
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
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
