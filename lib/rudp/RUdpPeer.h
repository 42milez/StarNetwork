#ifndef P2P_TECHDEMO_RUDPPEER_H
#define P2P_TECHDEMO_RUDPPEER_H

#include <list>
#include <memory>
#include <queue>
#include <tuple>

#include "core/errors.h"
#include "lib/rudp/command/RUdpCommand.h"
#include "lib/rudp/command/RUdpCommandPod.h"
#include "RUdpAcknowledgement.h"
#include "RUdpAddress.h"
#include "RUdpChamber.h"
#include "RUdpChannel.h"
#include "RUdpMacro.h"
#include "RUdpPeerNet.h"
#include "RUdpSegment.h"

class RUdpPeer
{
public:
    RUdpPeer();

    void ClearDispatchedCommandQueue();
    void Ping();

    bool
    EventOccur(const RUdpAddress &address, uint8_t session_id);

    bool
    LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, uint32_t service_time);

    bool
    LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber);

    std::shared_ptr<RUdpAcknowledgement>
    PopAcknowledgement();

    void
    QueueAcknowledgement(const RUdpProtocolType *cmd, uint16_t sent_time);

    void
    QueueOutgoingCommand(const std::shared_ptr<RUdpProtocolType> &command, const std::shared_ptr<RUdpSegment> &segment,
                         uint32_t offset, uint16_t length);

    std::tuple<std::shared_ptr<RUdpSegment>, uint8_t>
    Receive();

    RUdpProtocolCommand
    RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id);

    void
    Reset(uint16_t peer_idx);

    void
    ResetPeerQueues();

    Error
    Send(SysCh ch, const std::shared_ptr<RUdpSegment> &segment, bool checksum);

    Error
    Setup(const RUdpAddress &address, SysCh channel_count, uint32_t host_incoming_bandwidth,
          uint32_t host_outgoing_bandwidth, uint32_t data);

    void
    SetupConnectedPeer(const RUdpProtocolType *cmd, const RUdpAddress &received_address,
                       uint32_t host_incoming_bandwidth, uint32_t host_outgoing_bandwidth,
                       uint32_t channel_count);

    void
    UpdateRoundTripTimeVariance(uint32_t service_time, uint32_t round_trip_time);

    inline bool
    AcknowledgementExists() { return !acknowledgements_.empty(); }

    inline const RUdpAddress &
    Address() { return address_; };

    inline void
    Address(const RUdpAddress val) { address_ = val; }

    inline bool
    ChannelExists() { return !channels_.empty(); }

    inline void
    ClearAcknowledgement() { acknowledgements_.clear(); }

    inline void
    ClearChannel() { channels_.clear(); }

    inline bool
    Disconnected() { return net_->StateIs(RUdpPeerState::DISCONNECTED); }

    inline bool
    DispatchedCommandExists() { return !dispatched_commands_.empty(); }

    inline bool
    ExceedsMTU(size_t segment_size) { return net_->mtu() - segment_size >= sizeof(RUdpProtocolPing); }

    inline bool
    ExceedsPingInterval(uint32_t service_time) { return UDP_TIME_DIFFERENCE(service_time, last_receive_time_) >= ping_interval_; }

    inline void
    Reset() { this->Reset(incoming_peer_id_); }

    inline uint8_t
    StateAsNumber() { return static_cast<uint8_t>(net_->state()); }

public:
    inline const std::unique_ptr<RUdpCommandPod> &
    command_pod() { return command_pod_; };

    inline uint32_t
    connect_id() { return connect_id_; }

    inline uint32_t
    event_data() { return event_data_; };

    inline void
    event_data(uint32_t val) { event_data_ = val; };

    inline void
    incoming_session_id(uint8_t val) { incoming_session_id_ = val; }

    inline void
    last_receive_time(uint32_t val) { last_receive_time_ = val; }

    inline bool
    needs_dispatch() { return needs_dispatch_; };

    inline void
    needs_dispatch(bool val) { needs_dispatch_ = val; };

    inline const std::unique_ptr<RUdpPeerNet> &
    net() { return net_; };

    inline uint16_t
    outgoing_peer_id() { return outgoing_peer_id_; };

    inline void
    outgoing_peer_id(uint16_t val) { outgoing_peer_id_ = val; }

    inline uint8_t
    outgoing_session_id() { return outgoing_session_id_; };

    inline void
    outgoing_session_id(uint8_t val) { outgoing_session_id_ = val; }

private:
    std::list<std::shared_ptr<RUdpAcknowledgement>> acknowledgements_;
    std::vector<std::shared_ptr<RUdpChannel>> channels_;
    std::unique_ptr<RUdpCommandPod> command_pod_;
    std::vector<uint8_t> data_;
    std::queue<RUdpIncomingCommand> dispatched_commands_;
    std::unique_ptr<RUdpPeerNet> net_;
    std::array<uint32_t, PEER_UNSEQUENCED_WINDOW_SIZE / 32> unsequenced_windows_;

    RUdpAddress address_;

    size_t total_waiting_data_;

    uint16_t incoming_peer_id_;
    uint8_t incoming_session_id_;

    uint16_t outgoing_peer_id_;
    uint8_t outgoing_session_id_;

    uint32_t highest_round_trip_time_variance_;
    uint32_t last_receive_time_;
    uint32_t last_round_trip_time_;
    uint32_t last_round_trip_time_variance_;
    uint32_t lowest_round_trip_time_;

    uint32_t connect_id_;
    uint32_t event_data_;
    uint32_t ping_interval_;

    bool needs_dispatch_;
};

#endif // P2P_TECHDEMO_RUDPPEER_H
