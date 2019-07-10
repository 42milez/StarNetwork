#ifndef P2P_TECHDEMO_RUDPPEERNET_H
#define P2P_TECHDEMO_RUDPPEERNET_H

#include <cstdint>

#include "RUdpPeerState.h"

class RUdpPeerNet
{
public:
    RUdpPeerNet();

    bool StateIs(RUdpPeerState state);

    bool StateIsGreaterThanOrEqual(RUdpPeerState state);

    bool StateIsLessThanOrEqual(RUdpPeerState state);

    bool exceeds_segment_loss_interval(uint32_t service_time);

    void calculate_segment_loss(uint32_t service_time);

    void Reset();

    void setup();

    void increase_segments_lost(uint32_t val);

    void increase_segments_sent(uint32_t val);

    void update_segment_throttle_counter();

    bool exceeds_segment_throttle_counter();

public:
    void last_send_time(uint32_t val);

    uint32_t mtu();

    void state(const RUdpPeerState &state);

    uint32_t incoming_bandwidth();

    void incoming_bandwidth(uint32_t val);

    uint32_t incoming_bandwidth_throttle_epoch();

    void incoming_bandwidth_throttle_epoch(uint32_t val);

    uint32_t outgoing_bandwidth();

    void outgoing_bandwidth(uint32_t val);

    uint32_t outgoing_bandwidth_throttle_epoch();

    void outgoing_bandwidth_throttle_epoch(uint32_t val);

    uint32_t segment_loss_epoch();

    void segment_loss_epoch(uint32_t val);

    uint32_t segment_throttle();

    void segment_throttle(uint32_t val);

    uint32_t segment_throttle_limit();

    void segment_throttle_limit(uint32_t val);

    uint32_t segment_throttle_interval();

    void segment_throttle_interval(uint32_t val);

    uint32_t segment_throttle_acceleration();
    void segment_throttle_acceleration(uint32_t val);

    uint32_t segment_throttle_deceleration();
    void segment_throttle_deceleration(uint32_t val);

    uint32_t segments_sent();

    uint32_t window_size();

private:
    RUdpPeerState _state;

    uint32_t _incoming_bandwidth;

    uint32_t _incoming_bandwidth_throttle_epoch;

    uint32_t _last_send_time;

    uint32_t _mtu;

    uint32_t _outgoing_bandwidth;

    uint32_t _outgoing_bandwidth_throttle_epoch;

    uint32_t _segment_throttle;

    uint32_t _segment_throttle_acceleration;

    uint32_t _segment_throttle_counter;

    uint32_t _segment_throttle_deceleration;

    uint32_t _segment_throttle_epoch;

    uint32_t _segment_throttle_interval;

    uint32_t _segment_throttle_limit;

    uint32_t _segment_loss;

    uint32_t _segment_loss_epoch;

    uint32_t _segment_loss_variance;

    uint32_t _segments_lost;

    uint32_t _segments_sent;

    uint32_t _window_size;
};

#endif // P2P_TECHDEMO_RUDPPEERNET_H
