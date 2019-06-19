#include "RUdpDispatchQueue.h"

std::shared_ptr<RUdpPeer>
RUdpDispatchQueue::pop_peer()
{
    std::shared_ptr<RUdpPeer> peer = _queue.front();

    _queue.pop();

    return peer;
}

void
RUdpDispatchQueue::push(std::shared_ptr<RUdpPeer> &peer)
{
    _queue.push(peer);
}

bool
RUdpDispatchQueue::peer_exists()
{
    return !_queue.empty();
}
