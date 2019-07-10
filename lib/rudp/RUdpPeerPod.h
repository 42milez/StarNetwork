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
public:
    RUdpPeerPod(size_t peer_count, std::shared_ptr<RUdpConnection> &conn);

    std::shared_ptr<RUdpPeer> AvailablePeer();
    void BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth);
    EventStatus DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event);
    EventStatus ReceiveIncomingCommands(std::unique_ptr<RUdpEvent> &event);
    EventStatus SendOutgoingCommands(std::unique_ptr<RUdpEvent> &event, uint32_t service_time, bool check_for_timeouts);

public:
    using InterceptCallback = std::function<int (RUdpEvent &event)>;

private:
    std::vector<std::shared_ptr<RUdpPeer>> peers_;
    std::vector<uint8_t> received_data_;

    std::shared_ptr<RUdpCompressor> compressor_;
    std::shared_ptr<RUdpConnection> conn_;

    std::unique_ptr<RUdpProtocol> protocol_;

    ChecksumCallback checksum_;
    InterceptCallback intercept_;
    RUdpAddress received_address_;

    size_t duplicate_peers_;
    size_t peer_count_;
    size_t received_data_length_;

    uint32_t total_received_data_;
    uint32_t total_received_segments_;
    uint32_t total_sent_data_;
    uint32_t total_sent_segments_;

    std::vector<uint8_t> segment_data_1_; // TODO: up to PROTOCOL_MAXIMUM_MTU
    std::vector<uint8_t> segment_data_2_; // TODO: up to PROTOCOL_MAXIMUM_MTU
};

#endif // P2P_TECHDEMO_RUDPPEERPOD_H
