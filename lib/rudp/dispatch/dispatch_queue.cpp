#include "lib/rudp/dispatch/dispatch_queue.h"

namespace rudp
{
    std::shared_ptr<RUdpPeer>
    DispatchQueue::Dequeue()
    {
        std::shared_ptr<RUdpPeer> peer = queue_.front();

        queue_.pop();

        return peer;
    }

    void
    DispatchQueue::Enqueue(std::shared_ptr<RUdpPeer> &peer)
    {
        queue_.push(peer);
    }

    bool
    DispatchQueue::PeerExists()
    {
        return !queue_.empty();
    }
} // namespace rudp
