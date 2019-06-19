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
#include "RUdpPacket.h"
#include "RUdpPeerNet.h"

class RUdpPeer
{
private:
    std::unique_ptr<RUdpPeerNet> _net;

    uint16_t _outgoing_peer_id;

    uint16_t _incoming_peer_id;

    uint32_t _connect_id;

    uint8_t _outgoing_session_id;

    uint8_t _incoming_session_id;

    UdpAddress _address;

    void *_data;

    uint32_t _last_receive_time;

    uint32_t _ping_interval;

    uint32_t _last_round_trip_time;

    uint32_t _last_round_trip_time_variance;

    uint32_t _lowest_round_trip_time;

    uint32_t _highest_round_trip_time_variance;

    std::list<std::shared_ptr<UdpAcknowledgement>> _acknowledgements;

    std::queue<IncomingCommand> _dispatched_commands;

    bool _needs_dispatch;

    uint32_t _unsequenced_window[PEER_UNSEQUENCED_WINDOW_SIZE / 32];

    uint32_t _event_data;

    size_t _total_waiting_data;

    std::unique_ptr<RUdpCommandPod> _command_pod;

    std::vector<std::shared_ptr<UdpChannel>> _channels;

public:
    RUdpPeer();

    void queue_outgoing_command(const std::shared_ptr<RUdpProtocolType> &command, const std::shared_ptr<RUdpPacket> &packet, uint32_t offset, uint16_t length);

    bool is_disconnected();

    Error
    setup(const UdpAddress &address, SysCh channel_count, uint32_t data, uint32_t in_bandwidth, uint32_t out_bandwidth);

    void udp_peer_disconnect();

    std::shared_ptr<RUdpPacket> udp_peer_receive(uint8_t &channel_id);

    void udp_peer_ping();

    bool needs_dispatch();

    bool needs_dispatch(bool val);

    bool acknowledgement_exists();

    bool dispatched_command_exists();

    void clear_dispatched_command();

    std::shared_ptr<UdpAcknowledgement> pop_acknowledgement();

    uint32_t mtu();

    uint32_t event_data();

    void event_data(uint32_t val);

    bool load_reliable_commands_into_chamber(std::unique_ptr<UdpChamber> &chamber, uint32_t service_time);

    bool load_unreliable_commands_into_chamber(std::unique_ptr<UdpChamber> &chamber);

    bool state_is(RUdpPeerState state);

    bool state_is_ge(RUdpPeerState state);

    bool state_is_lt(RUdpPeerState state);

    uint32_t outgoing_data_total();

    void outgoing_data_total(uint32_t val);

    uint32_t incoming_data_total();

    void incoming_data_total(uint32_t val);

    uint32_t incoming_bandwidth();

    uint32_t outgoing_bandwidth_throttle_epoch();

    void outgoing_bandwidth_throttle_epoch(uint32_t val);

    uint32_t packet_throttle_limit();

    void packet_throttle_limit(uint32_t val);

    uint32_t packet_throttle();

    void packet_throttle(uint32_t val);

    const std::unique_ptr<RUdpPeerNet> &net();

    const std::unique_ptr<RUdpCommandPod> &command();

    bool exceeds_ping_interval(uint32_t service_time);

    bool exceeds_mtu(size_t packet_size);

    uint16_t outgoing_peer_id();

    uint8_t outgoing_session_id();

    const UdpAddress & address();

    void reset();

    void clear_acknowledgement();

    bool channel_exists();

    void clear_channel();
};

#endif // P2P_TECHDEMO_RUDPPEER_H
