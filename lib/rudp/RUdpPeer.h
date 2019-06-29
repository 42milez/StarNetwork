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

    void Disconnect();

    Error Setup(const RUdpAddress &address, SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth,
                uint32_t data);

    void Ping();

    void QueueOutgoingCommand(std::shared_ptr<RUdpProtocolType> &command,
                              std::shared_ptr<RUdpSegment> &segment,
                              uint32_t offset,
                              uint16_t length);

    std::shared_ptr<RUdpSegment> Receive(uint8_t &channel_id);

public:
    bool AcknowledgementExists();
    void ClearAcknowledgement();
    std::shared_ptr<RUdpAcknowledgement> PopAcknowledgement();

    bool ChannelExists();
    void ClearChannel();

    void ClearDispatchedCommandQueue();
    bool DispatchedCommandExists();

    bool Disconnected();

    bool ExceedsMTU(size_t segment_size);

    bool ExceedsPingInterval(uint32_t service_time);

    bool LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, uint32_t service_time);

    bool LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber);

    void Reset();

    bool StateIs(RUdpPeerState state);
    bool StateIsGreaterThanOrEqual(RUdpPeerState state);
    bool StateIsLessThanOrEqual(RUdpPeerState state);

public:
    inline uint32_t event_data()
    { return event_data_; };

    inline void event_data(uint32_t val)
    { event_data_ = val; };

    inline uint32_t incoming_bandwidth()
    { return net_->incoming_bandwidth(); };

    inline uint32_t incoming_data_total()
    { return command_pod_->incoming_data_total(); };

    inline void incoming_data_total(uint32_t val)
    { command_pod_->incoming_data_total(val); };

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

    inline uint8_t outgoing_session_id()
    { return outgoing_session_id_; };

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

    inline const std::unique_ptr<RUdpCommandPod> &command()
    { return command_pod_; };

    inline const std::unique_ptr<RUdpPeerNet> &net()
    { return net_; };

private:
    std::list<std::shared_ptr<RUdpAcknowledgement>> acknowledgements_;

    std::queue<IncomingCommand> dispatched_commands_;

    std::vector<std::shared_ptr<RUdpChannel>> channels_;

    std::unique_ptr<RUdpCommandPod> command_pod_;
    std::unique_ptr<RUdpPeerNet> net_;

    RUdpAddress address_;

    size_t total_waiting_data_;

    uint32_t connect_id_;
    uint32_t event_data_;
    uint32_t highest_round_trip_time_variance_;
    uint32_t last_receive_time_;
    uint32_t last_round_trip_time_;
    uint32_t last_round_trip_time_variance_;
    uint32_t lowest_round_trip_time_;
    uint32_t ping_interval_;
    uint32_t unsequenced_windows_[PEER_UNSEQUENCED_WINDOW_SIZE / 32];

    uint16_t incoming_peer_id_;
    uint16_t outgoing_peer_id_;

    uint8_t incoming_session_id_;
    uint8_t outgoing_session_id_;

    bool needs_dispatch_;

    void *data_;
};

#endif // P2P_TECHDEMO_RUDPPEER_H
