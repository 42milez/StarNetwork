#ifndef P2P_TECHDEMO_RUDPPEERPOD_H
#define P2P_TECHDEMO_RUDPPEERPOD_H

#include <vector>

#include "lib/rudp/protocol/RUdpProtocol.h"
#include "RUdpChecksum.h"
#include "RUdpCompress.h"
#include "RUdpConnection.h"
#include "RUdpPeer.h"
#include "RUdpTime.h"

using InterceptCallback = std::function<int (RUdpEvent &event)>;

class RUdpPeerPod
{
public:
    RUdpPeerPod(size_t peer_count,
                std::shared_ptr<RUdpConnection> &conn,
                uint32_t host_incoming_bandwidth,
                uint32_t host_outgoing_bandwidth);

    std::shared_ptr<RUdpPeer>
    AvailablePeer();

    Error
    Disconnect(const std::shared_ptr<RUdpPeer> &peer, uint32_t data);

    Error
    DisconnectLater(const std::shared_ptr<RUdpPeer> &peer, uint32_t data);

    Error
    DisconnectNow(const std::shared_ptr<RUdpPeer> &peer, uint32_t data);

    EventStatus
    ReceiveIncomingCommands(std::unique_ptr<RUdpEvent> &event);

    EventStatus
    SendOutgoingCommands(const std::unique_ptr<RUdpEvent> &event, uint32_t service_time, bool check_for_timeouts);

    void
    RequestPeerRemoval(size_t peer_idx, const std::shared_ptr<RUdpPeer> &peer);

    void
    Flush();

    inline void
    BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth)
    { protocol_->BandwidthThrottle(service_time, incoming_bandwidth, outgoing_bandwidth, peers_); }

    inline EventStatus
    DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event) { return protocol_->DispatchIncomingCommands(event); }

    inline std::shared_ptr<RUdpPeer>
    Peer(size_t peer_idx) { return peers_.at(peer_idx); }

    inline void
    PeerOnDisconnect(const std::shared_ptr<RUdpPeer> &peer) { protocol_->PeerOnDisconnect(peer); }

public:
    inline uint32_t
    service_time() { return service_time_; }

    inline void
    update_service_time() { service_time_ = RUdpTime::Get(); }

private:
    std::shared_ptr<RUdpCompress> compressor_;
    std::shared_ptr<RUdpConnection> conn_;
    std::vector<std::shared_ptr<RUdpPeer>> peers_;
    std::unique_ptr<RUdpProtocol> protocol_;
    VecUInt8Ptr received_data_;
    std::vector<uint8_t> segment_data_1_; // TODO: up to PROTOCOL_MAXIMUM_MTU
    std::vector<uint8_t> segment_data_2_; // TODO: up to PROTOCOL_MAXIMUM_MTU

    ChecksumCallback checksum_;
    InterceptCallback intercept_;
    RUdpAddress received_address_;

    size_t duplicate_peers_;
    size_t peer_count_;
    size_t received_data_length_;

    uint32_t host_incoming_bandwidth_;
    uint32_t host_outgoing_bandwidth_;
    uint32_t service_time_;
    uint32_t total_received_data_;
    uint32_t total_received_segments_;
    uint32_t total_sent_data_;
    uint32_t total_sent_segments_;
};

#endif // P2P_TECHDEMO_RUDPPEERPOD_H
