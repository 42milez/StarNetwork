#include "udp.h"

std::shared_ptr<UdpPeer>
UdpDispatchQueue::pop_peer()
{
    std::shared_ptr<UdpPeer> peer = _queue.front();

    _queue.pop();

    return peer;
}

void
UdpDispatchQueue::push(std::shared_ptr<UdpPeer> &peer)
{
    _queue.push(peer);
}

bool
UdpDispatchQueue::peer_exists()
{
    return !_queue.empty();
}
