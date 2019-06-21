#ifndef P2P_TECHDEMO_RUDPPEERPOD_H
#define P2P_TECHDEMO_RUDPPEERPOD_H

#include <vector>

#include "RUdpChecksum.h"
#include "RUdpCompressor.h"
#include "RUdpConnection.h"
#include "RUdpPeer.h"
#include "RUdpProtocol.h"

class RUdpPeerPod
{
private:
    std::vector<std::shared_ptr<RUdpPeer>> _peers;

    std::unique_ptr<RUdpProtocol> _protocol;

    size_t _peer_count;

    ChecksumCallback _checksum;

    std::shared_ptr<RUdpCompressor> _compressor;

    uint32_t _total_sent_data;

    uint32_t _total_sent_segments;

    uint32_t _total_received_data;

    uint32_t _total_received_segments;

    std::shared_ptr<RUdpConnection> _conn;

public:
    RUdpPeerPod(size_t peer_count, std::shared_ptr<RUdpConnection> &conn);

    std::shared_ptr<RUdpPeer> available_peer_exists();

    int send_outgoing_commands(std::unique_ptr<RUdpEvent> &event, uint32_t service_time, bool check_for_timeouts);

    int protocol_dispatch_incoming_commands(std::unique_ptr<RUdpEvent> &event);

    void protocol_bandwidth_throttle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth);

    std::unique_ptr<RUdpProtocol> &protocol();
};

#endif // P2P_TECHDEMO_RUDPPEERPOD_H
