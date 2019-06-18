#ifndef P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
#define P2P_TECHDEMO_RUDPDISPATCHQUEUE_H

class UdpDispatchQueue
{
private:
    std::queue<std::shared_ptr<UdpPeer>> _queue;

public:
    std::shared_ptr<UdpPeer> pop_peer();

    void push(std::shared_ptr<UdpPeer> &peer);

    bool peer_exists();
};

#endif // P2P_TECHDEMO_RUDPDISPATCHQUEUE_H
