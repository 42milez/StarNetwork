#ifndef P2P_TECHDEMO_UDP_DISPATCH_QUEUE_H
#define P2P_TECHDEMO_UDP_DISPATCH_QUEUE_H

class UdpDispatchQueue
{
private:
    std::queue<std::shared_ptr<UdpPeer>> _queue;

public:
    std::shared_ptr<UdpPeer> pop_peer();

    void push(std::shared_ptr<UdpPeer> &peer);

    bool peer_exists();
};

#endif // P2P_TECHDEMO_UDP_DISPATCH_QUEUE_H
