#include "lib/rudp/dispatch/dispatch_queue.h"

namespace rudp
{
    std::shared_ptr<Peer>
    DispatchQueue::Dequeue()
    {
        std::shared_ptr<Peer> peer = queue_.front();

        queue_.pop();

        return peer;
    }

    void
    DispatchQueue::Enqueue(std::shared_ptr<Peer>& peer)
    {
        queue_.push(peer);
    }

    bool
    DispatchQueue::PeerExists()
    {
        return !queue_.empty();
    }
} // namespace rudp
