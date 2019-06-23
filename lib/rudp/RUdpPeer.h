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

    Error Setup(const RUdpAddress &address, SysCh channel_count, uint32_t data, uint32_t in_bandwidth,
                uint32_t out_bandwidth);

    void Ping();

    void QueueOutgoingCommand(const std::shared_ptr<RUdpProtocolType> &command,
                              const std::shared_ptr<RUdpSegment> &segment,
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

    bool LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, uint32_t service_time);

    bool LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber);

    void Reset();

    bool StateIs(RUdpPeerState state);
    bool StateIsGreaterThanOrEqual(RUdpPeerState state);
    bool StateIsLessThanOrEqual(RUdpPeerState state);

public:
    uint32_t event_data();
    void event_data(uint32_t val);

    uint32_t incoming_bandwidth();

    uint32_t incoming_data_total();
    void incoming_data_total(uint32_t val);

    bool needs_dispatch();
    void needs_dispatch(bool val);

    uint32_t outgoing_bandwidth_throttle_epoch();
    void outgoing_bandwidth_throttle_epoch(uint32_t val);

    uint32_t outgoing_data_total();
    void outgoing_data_total(uint32_t val);

    uint16_t outgoing_peer_id();
    uint8_t outgoing_session_id();

    uint32_t segment_throttle();
    void segment_throttle(uint32_t val);
    uint32_t segment_throttle_limit();
    void segment_throttle_limit(uint32_t val);

    bool exceeds_mtu(size_t segment_size);
    bool exceeds_ping_interval(uint32_t service_time);
    
public:
    const RUdpAddress &address();
    const std::unique_ptr<RUdpCommandPod> &command();
    const std::unique_ptr<RUdpPeerNet> &net();

private:
    std::list<std::shared_ptr<RUdpAcknowledgement>> _acknowledgements;

    std::queue<IncomingCommand> _dispatched_commands;

    std::vector<std::shared_ptr<RUdpChannel>> _channels;

    std::unique_ptr<RUdpCommandPod> _command_pod;
    std::unique_ptr<RUdpPeerNet> _net;

    RUdpAddress _address;

    size_t _total_waiting_data;

    uint32_t _connect_id;
    uint32_t _event_data;
    uint32_t _highest_round_trip_time_variance;
    uint32_t _last_receive_time;
    uint32_t _last_round_trip_time;
    uint32_t _last_round_trip_time_variance;
    uint32_t _lowest_round_trip_time;
    uint32_t _ping_interval;
    uint32_t _unsequenced_window[PEER_UNSEQUENCED_WINDOW_SIZE / 32];

    uint16_t _incoming_peer_id;
    uint16_t _outgoing_peer_id;

    uint8_t _incoming_session_id;
    uint8_t _outgoing_session_id;

    bool _needs_dispatch;

    void *_data;
};

#endif // P2P_TECHDEMO_RUDPPEER_H
