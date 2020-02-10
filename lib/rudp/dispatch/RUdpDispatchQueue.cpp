#include "lib/rudp/dispatch/RUdpDispatchQueue.h"

namespace rudp
{
    std::shared_ptr<RUdpPeer>
    RUdpDispatchQueue::Dequeue()
    {
        std::shared_ptr<RUdpPeer> peer = queue_.front();

        queue_.pop();

        return peer;
    }

    void
    RUdpDispatchQueue::Enqueue(std::shared_ptr<RUdpPeer> &peer)
    {
        queue_.push(peer);
    }

    bool
    RUdpDispatchQueue::PeerExists()
    {
        return !queue_.empty();
    }
} // namespace rudp
