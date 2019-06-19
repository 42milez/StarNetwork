#include "RUdpDispatchQueue.h"

std::shared_ptr<RUdpPeer>
UdpDispatchQueue::pop_peer()
{
    std::shared_ptr<RUdpPeer> peer = _queue.front();

    _queue.pop();

    return peer;
}

void
UdpDispatchQueue::push(std::shared_ptr<RUdpPeer> &peer)
{
    _queue.push(peer);
}

bool
UdpDispatchQueue::peer_exists()
{
    return !_queue.empty();
}
