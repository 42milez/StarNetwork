#include "RUdpCommon.h"
#include "RUdpPeerNet.h"

RUdpPeerNet::RUdpPeerNet() : _state(RUdpPeerState::DISCONNECTED),
                             _segment_throttle(0),
                             _segment_throttle_limit(0),
                             _segment_throttle_counter(0),
                             _segment_throttle_epoch(0),
                             _segment_throttle_acceleration(0),
                             _segment_throttle_deceleration(0),
                             _segment_throttle_interval(0),
                             _segment_loss_epoch(0),
                             _segments_lost(0),
                             _segment_loss(0),
                             _segment_loss_variance(0),
                             _last_send_time(0),
                             _mtu(0),
                             _window_size(0),
                             _incoming_bandwidth(0),
                             _outgoing_bandwidth(0),
                             _incoming_bandwidth_throttle_epoch(0),
                             _outgoing_bandwidth_throttle_epoch(0),
                             _segments_sent(0)
{}

void
RUdpPeerNet::setup()
{
    _state = RUdpPeerState::CONNECTING;

    if (_outgoing_bandwidth == 0)
        _window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
        _window_size = (_outgoing_bandwidth / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (_window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
        _window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (_window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
        _window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
}

void
RUdpPeerNet::state(const RUdpPeerState &state)
{
    _state = state;
}

uint32_t
RUdpPeerNet::mtu()
{
    return _mtu;
}

bool
RUdpPeerNet::state_is(RUdpPeerState state)
{
    return _state == state;
}

bool
RUdpPeerNet::state_is_ge(RUdpPeerState state)
{
    return _state >= state;
}

bool
RUdpPeerNet::state_is_lt(RUdpPeerState state)
{
    return _state < state;
}

uint32_t
RUdpPeerNet::incoming_bandwidth()
{
    return _incoming_bandwidth;
}

uint32_t
RUdpPeerNet::outgoing_bandwidth()
{
    return _outgoing_bandwidth;
}

uint32_t
RUdpPeerNet::incoming_bandwidth_throttle_epoch()
{
    return _incoming_bandwidth_throttle_epoch;
}

void
RUdpPeerNet::incoming_bandwidth_throttle_epoch(uint32_t val)
{
    _incoming_bandwidth_throttle_epoch = val;
}

uint32_t
RUdpPeerNet::outgoing_bandwidth_throttle_epoch()
{
    return _outgoing_bandwidth_throttle_epoch;
}

void
RUdpPeerNet::outgoing_bandwidth_throttle_epoch(uint32_t val)
{
    _outgoing_bandwidth_throttle_epoch = val;
}

uint32_t
RUdpPeerNet::segment_throttle_limit()
{
    return _segment_throttle_limit;
}

void
RUdpPeerNet::segment_throttle_limit(uint32_t val)
{
    _segment_throttle_limit = val;
}

uint32_t
RUdpPeerNet::segment_throttle()
{
    return _segment_throttle;
}

void
RUdpPeerNet::segment_throttle(uint32_t val)
{
    _segment_throttle = val;
}

uint32_t
RUdpPeerNet::segment_loss_epoch()
{
    return _segment_loss_epoch;
}

void
RUdpPeerNet::segment_loss_epoch(uint32_t val)
{
    _segment_loss_epoch = val;
}

bool
RUdpPeerNet::exceeds_segment_loss_interval(uint32_t service_time)
{
    return UDP_TIME_DIFFERENCE(service_time, _segment_loss_epoch) >= PEER_SEGMENT_LOSS_INTERVAL;
}

uint32_t
RUdpPeerNet::segments_sent()
{
    return _segments_sent;
}

void
RUdpPeerNet::calculate_segment_loss(uint32_t service_time)
{
    uint32_t segment_loss = _segments_lost * PEER_SEGMENT_LOSS_SCALE / _segments_sent;

    _segment_loss_variance -= _segment_loss_variance / 4;

    if (segment_loss >= _segment_loss)
    {
        _segment_loss += (segment_loss - _segment_loss) / 8;
        _segment_loss_variance += (segment_loss - _segment_loss) / 4;
    }
    else
    {
        _segment_loss -= (_segment_loss - segment_loss) / 8;
        _segment_loss_variance += (_segment_loss - segment_loss) / 4;
    }

    _segment_loss_epoch = service_time;
    _segments_sent = 0;
    _segments_lost = 0;
}

void
RUdpPeerNet::last_send_time(uint32_t service_time)
{
    _last_send_time = service_time;
}

void
RUdpPeerNet::reset()
{
    _state = RUdpPeerState::DISCONNECTED;
    _last_send_time = 0;
    _segment_throttle = PEER_DEFAULT_SEGMENT_THROTTLE;
    _segment_throttle_limit = PEER_SEGMENT_THROTTLE_SCALE;
    _segment_throttle_counter = 0;
    _segment_throttle_epoch = 0;
    _segment_throttle_acceleration = PEER_SEGMENT_THROTTLE_ACCELERATION;
    _segment_throttle_deceleration = PEER_SEGMENT_THROTTLE_DECELERATION;
    _segment_throttle_interval = PEER_SEGMENT_THROTTLE_INTERVAL;
    _incoming_bandwidth = 0;
    _outgoing_bandwidth = 0;
    _incoming_bandwidth_throttle_epoch = 0;
    _outgoing_bandwidth_throttle_epoch = 0;
    _segment_loss_epoch = 0;
    _segments_sent = 0;
    _segments_lost = 0;
    _segment_loss = 0;
    _segment_loss_variance = 0;
    _mtu = HOST_DEFAULT_MTU;
    _window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
}

uint32_t
RUdpPeerNet::window_size()
{
    return _window_size;
}

uint32_t
RUdpPeerNet::segment_throttle_interval()
{
    return _segment_throttle_interval;
}

uint32_t
RUdpPeerNet::segment_throttle_acceleration()
{
    return _segment_throttle_acceleration;
}

uint32_t
RUdpPeerNet::segment_throttle_deceleration()
{
    return _segment_throttle_deceleration;
}

void
RUdpPeerNet::increase_segments_lost(uint32_t val)
{
    _segments_lost += val;
}

void
RUdpPeerNet::increase_segments_sent(uint32_t val)
{
    _segments_sent += val;
}

void
RUdpPeerNet::update_segment_throttle_counter()
{
    _segment_throttle_counter += PEER_SEGMENT_THROTTLE_COUNTER;
    _segment_throttle_counter %= PEER_SEGMENT_THROTTLE_SCALE;
}

bool
RUdpPeerNet::exceeds_segment_throttle_counter()
{
    return _segment_throttle_counter > _segment_throttle;
}
