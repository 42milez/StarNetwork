#ifndef P2P_TECHDEMO_RUDPPEERNET_H
#define P2P_TECHDEMO_RUDPPEERNET_H

#include <cstdint>

#include "RUdpPeerState.h"

class UdpPeerNet
{
private:
    UdpPeerState _state;

    uint32_t _packet_throttle;

    uint32_t _packet_throttle_limit;

    uint32_t _packet_throttle_counter;

    uint32_t _packet_throttle_epoch;

    uint32_t _packet_throttle_acceleration;

    uint32_t _packet_throttle_deceleration;

    uint32_t _packet_throttle_interval;

    uint32_t _mtu;

    uint32_t _window_size;

    uint32_t _incoming_bandwidth;

    uint32_t _outgoing_bandwidth;

    uint32_t _incoming_bandwidth_throttle_epoch;

    uint32_t _outgoing_bandwidth_throttle_epoch;

    uint32_t _packet_loss_epoch;

    uint32_t _packets_lost;

    uint32_t _packet_loss;

    uint32_t _packet_loss_variance;

    uint32_t _packets_sent;

    uint32_t _last_send_time;

public:
    UdpPeerNet();

    uint32_t mtu();

    void state(const UdpPeerState &state);

    bool state_is(UdpPeerState state);

    bool state_is_ge(UdpPeerState state);

    bool state_is_lt(UdpPeerState state);

    uint32_t incoming_bandwidth();

    uint32_t outgoing_bandwidth();

    uint32_t incoming_bandwidth_throttle_epoch();

    void incoming_bandwidth_throttle_epoch(uint32_t val);

    uint32_t outgoing_bandwidth_throttle_epoch();

    void outgoing_bandwidth_throttle_epoch(uint32_t val);

    uint32_t packet_throttle();

    void packet_throttle(uint32_t val);

    uint32_t packet_throttle_limit();

    void packet_throttle_limit(uint32_t val);

    uint32_t packet_loss_epoch();

    void packet_loss_epoch(uint32_t val);

    uint32_t packets_lost();

    uint32_t packet_loss();

    uint32_t packet_loss_variance();

    void packet_loss_variance(uint32_t val);

    bool exceeds_packet_loss_interval(uint32_t service_time);

    uint32_t packets_sent();

    void calculate_packet_loss(uint32_t service_time);

    void last_send_time(uint32_t val);

    void reset();

    uint32_t window_size();

    void window_size(uint32_t val);

    void setup();

    uint32_t packet_throttle_interval();

    uint32_t packet_throttle_acceleration();

    uint32_t packet_throttle_deceleration();

    void increase_packets_lost(uint32_t val);

    void increase_packets_sent(uint32_t val);

    void update_packet_throttle_counter();

    bool exceeds_packet_throttle_counter();
};

#endif // P2P_TECHDEMO_RUDPPEERNET_H
