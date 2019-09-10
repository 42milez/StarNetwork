#ifndef P2P_TECHDEMO_RUDPPEER_H
#define P2P_TECHDEMO_RUDPPEER_H

#include <list>
#include <memory>
#include <queue>

#include "RUdpAddress.h"
#include "RUdpAcknowledgement.h"
#include "RUdpChamber.h"
#include "RUdpChannel.h"
#include "RUdpCommand.h"
#include "RUdpCommandPod.h"
#include "RUdpCommon.h"
#include "RUdpSegment.h"
#include "RUdpPeerNet.h"

class RUdpPeer
{
public:
    RUdpPeer();

    Error Setup(const RUdpAddress &address, SysCh channel_count, uint32_t host_incoming_bandwidth,
                uint32_t host_outgoing_bandwidth, uint32_t data);

    void SetupConnectedPeer(const RUdpProtocolType *cmd,
                            const RUdpAddress &received_address,
                            uint32_t host_incoming_bandwidth,
                            uint32_t host_outgoing_bandwidth,
                            uint32_t channel_count);

    void Ping();

    void QueueAcknowledgement(const RUdpProtocolType *cmd, uint16_t sent_time);

    void QueueOutgoingCommand(const std::shared_ptr<RUdpProtocolType> &command,
                              const std::shared_ptr<RUdpSegment> &segment,
                              uint32_t offset,
                              uint16_t length);

    std::shared_ptr<RUdpSegment> Receive(uint8_t &channel_id);
    Error Send(SysCh ch, const std::shared_ptr<RUdpSegment> &segment, bool checksum);

public:
    bool AcknowledgementExists();
    void ClearAcknowledgement();
    std::shared_ptr<RUdpAcknowledgement> PopAcknowledgement();

    bool ChannelExists();
    void ClearChannel();

    void ClearDispatchedCommandQueue();
    bool DispatchedCommandExists();

    bool Disconnected();

    bool EventOccur(const RUdpAddress &address, uint8_t session_id);

    bool ExceedsMTU(size_t segment_size);
    bool ExceedsPingInterval(uint32_t service_time);

    bool LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, uint32_t service_time);
    bool LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber);

    void Reset();
    void Reset(uint16_t peer_idx);
    void ResetPeerQueues();

    inline uint8_t StateAsNumber()
    { return static_cast<uint8_t>(net_->state()); }

    bool StateIs(RUdpPeerState state);
    bool StateIsGreaterThanOrEqual(RUdpPeerState state);
    bool StateIsLessThanOrEqual(RUdpPeerState state);

    void UpdateRoundTripTimeVariance(uint32_t service_time, uint32_t round_trip_time);

    RUdpProtocolCommand RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id);

public:
    inline size_t channel_count()
    { return channels_.size(); }

    inline uint32_t connect_id()
    { return connect_id_; }

    inline void connect_id(uint32_t val)
    { connect_id_ = val; }

    inline void earliest_timeout(uint32_t val)
    { earliest_timeout_ = val; }

    inline uint32_t event_data()
    { return event_data_; };

    inline void event_data(uint32_t val)
    { event_data_ = val; };

    inline uint32_t incoming_bandwidth()
    { return net_->incoming_bandwidth(); };

    inline void incoming_bandwidth(uint32_t val)
    { net_->incoming_bandwidth(val); }

    inline void outgoing_bandwidth(uint32_t val)
    { net_->outgoing_bandwidth(val); }

    inline uint32_t incoming_data_total()
    { return command_pod_->incoming_data_total(); };

    inline void incoming_data_total(uint32_t val)
    { command_pod_->incoming_data_total(val); };

    inline void last_receive_time(uint32_t val)
    { last_receive_time_ = val; }

    inline uint32_t mtu()
    { return net_->mtu(); }

    inline void mtu(uint32_t val)
    { net_->mtu(val); }

    inline bool needs_dispatch()
    { return needs_dispatch_; };

    inline void needs_dispatch(bool val)
    { needs_dispatch_ = val; };

    inline uint32_t outgoing_bandwidth_throttle_epoch()
    { return net_->outgoing_bandwidth_throttle_epoch(); };

    inline void outgoing_bandwidth_throttle_epoch(uint32_t val)
    { net_->outgoing_bandwidth_throttle_epoch(val); };

    inline uint32_t outgoing_data_total()
    { return command_pod_->outgoing_data_total(); };

    inline void outgoing_data_total(uint32_t val)
    { command_pod_->outgoing_data_total(val); };

    inline uint16_t outgoing_peer_id()
    { return outgoing_peer_id_; };

    inline void outgoing_peer_id(uint16_t val)
    { outgoing_peer_id_ = val; }

    inline uint16_t incoming_peer_id()
    { return incoming_peer_id_; }

    inline void incoming_session_id(uint8_t val)
    { incoming_session_id_ = val; }

    inline uint8_t outgoing_session_id()
    { return outgoing_session_id_; };

    inline void outgoing_session_id(uint8_t val)
    { outgoing_session_id_ = val; }

    inline uint32_t segment_throttle()
    { return net_->segment_throttle(); };

    inline void segment_throttle(uint32_t val)
    { net_->segment_throttle(val); };

    inline uint32_t segment_throttle_limit()
    { return net_->segment_throttle_limit(); };

    inline void segment_throttle_limit(uint32_t val)
    { net_->segment_throttle_limit(val); };

public:
    inline const RUdpAddress &address()
    { return address_; };

    inline void address(const RUdpAddress &address)
    {
        address_ = address;
    }

    inline std::shared_ptr<RUdpChannel> channel(size_t channel_id)
    { return channels_.at(channel_id); }

    inline const std::unique_ptr<RUdpCommandPod> &command()
    { return command_pod_; };

    inline const std::unique_ptr<RUdpPeerNet> &net()
    { return net_; };

private:
    std::list<std::shared_ptr<RUdpAcknowledgement>> acknowledgements_;
    std::vector<std::shared_ptr<RUdpChannel>> channels_;
    std::unique_ptr<RUdpCommandPod> command_pod_;
    std::queue<IncomingCommand> dispatched_commands_;
    std::unique_ptr<RUdpPeerNet> net_;

    uint16_t incoming_peer_id_;
    uint8_t incoming_session_id_;

    uint16_t outgoing_peer_id_;
    uint8_t outgoing_session_id_;

    uint32_t earliest_timeout_;
    uint32_t highest_round_trip_time_variance_;
    uint32_t last_receive_time_;
    uint32_t last_round_trip_time_;
    uint32_t last_round_trip_time_variance_;
    uint32_t lowest_round_trip_time_;

    RUdpAddress address_;
    uint32_t connect_id_;
    std::vector<uint8_t> data_;
    uint32_t event_data_;
    bool needs_dispatch_;
    uint32_t ping_interval_;
    size_t total_waiting_data_;
    std::array<uint32_t, PEER_UNSEQUENCED_WINDOW_SIZE / 32> unsequenced_windows_;
};

#endif // P2P_TECHDEMO_RUDPPEER_H
