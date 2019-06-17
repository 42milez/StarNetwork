#ifndef P2P_TECHDEMO_UDP_PEER_POD_H
#define P2P_TECHDEMO_UDP_PEER_POD_H

class UdpPeerPod
{
private:
    std::vector<std::shared_ptr<UdpPeer>> _peers;

    std::unique_ptr<UdpProtocol> _protocol;

    size_t _peer_count;

    UdpChecksumCallback _checksum;

    std::shared_ptr<UdpCompressor> _compressor;

    std::shared_ptr<UdpHost> _host;

    uint32_t _total_sent_data;

    uint32_t _total_sent_packets;

    uint32_t _total_received_data;

    uint32_t _total_received_packets;

public:
    UdpPeerPod(size_t peer_count);

    std::shared_ptr<UdpPeer> available_peer_exists();

    void bandwidth_throttle(uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth);

    int send_outgoing_commands(std::unique_ptr<UdpEvent> &event, uint32_t service_time, bool check_for_timeouts);

    int protocol_dispatch_incoming_commands(std::unique_ptr<UdpEvent> &event);

    void protocol_bandwidth_throttle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth);

    std::unique_ptr<UdpProtocol> &protocol();
};

#endif // P2P_TECHDEMO_UDP_PEER_POD_H
