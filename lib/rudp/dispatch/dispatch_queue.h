#ifndef P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
#define P2P_TECHDEMO_RUDPDISPATCHQUEUE_H

#include <memory>
#include <queue>

#include "lib/rudp/peer/peer.h"

namespace rudp
{
    class DispatchQueue
    {
      public:
        void
        Enqueue(std::shared_ptr<Peer> &peer);

        std::shared_ptr<Peer>
        Dequeue();

      public:
        bool
        PeerExists();

      private:
        std::queue<std::shared_ptr<Peer>> queue_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
