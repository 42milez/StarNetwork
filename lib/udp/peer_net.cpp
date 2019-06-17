#include "peer_net.h"

UdpPeerNet::UdpPeerNet() : _state(UdpPeerState::DISCONNECTED),
                           _packet_throttle(0),
                           _packet_throttle_limit(0),
                           _packet_throttle_counter(0),
                           _packet_throttle_epoch(0),
                           _packet_throttle_acceleration(0),
                           _packet_throttle_deceleration(0),
                           _packet_throttle_interval(0),
                           _packet_loss_epoch(0),
                           _packets_lost(0),
                           _packet_loss(0),
                           _packet_loss_variance(0),
                           _last_send_time(0),
                           _mtu(0),
                           _window_size(0),
                           _incoming_bandwidth(0),
                           _outgoing_bandwidth(0),
                           _incoming_bandwidth_throttle_epoch(0),
                           _outgoing_bandwidth_throttle_epoch(0),
                           _packets_sent(0)
{}

void
UdpPeerNet::setup()
{
    _state = UdpPeerState::CONNECTING;

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
UdpPeerNet::state(const UdpPeerState &state)
{
    _state = state;
}

uint32_t
UdpPeerNet::mtu()
{
    return _mtu;
}

bool
UdpPeerNet::state_is(UdpPeerState state)
{
    return _state == state;
}

bool
UdpPeerNet::state_is_ge(UdpPeerState state)
{
    return _state >= state;
}

bool
UdpPeerNet::state_is_lt(UdpPeerState state)
{
    return _state < state;
}

uint32_t
UdpPeerNet::incoming_bandwidth()
{
    return _incoming_bandwidth;
}

uint32_t
UdpPeerNet::outgoing_bandwidth()
{
    return _outgoing_bandwidth;
}

uint32_t
UdpPeerNet::incoming_bandwidth_throttle_epoch()
{
    return _incoming_bandwidth_throttle_epoch;
}

void
UdpPeerNet::incoming_bandwidth_throttle_epoch(uint32_t val)
{
    _incoming_bandwidth_throttle_epoch = val;
}


uint32_t
UdpPeerNet::outgoing_bandwidth_throttle_epoch()
{
    return _outgoing_bandwidth_throttle_epoch;
}

void
UdpPeerNet::outgoing_bandwidth_throttle_epoch(uint32_t val)
{
    _outgoing_bandwidth_throttle_epoch = val;
}

uint32_t
UdpPeerNet::packet_throttle_limit()
{
    return _packet_throttle_limit;
}

void
UdpPeerNet::packet_throttle_limit(uint32_t val)
{
    _packet_throttle_limit = val;
}

uint32_t
UdpPeerNet::packet_throttle()
{
    return _packet_throttle;
}

void
UdpPeerNet::packet_throttle(uint32_t val)
{
    _packet_throttle = val;
}

uint32_t
UdpPeerNet::packet_loss_epoch()
{
    return _packet_loss_epoch;
}

void
UdpPeerNet::packet_loss_epoch(uint32_t val)
{
    _packet_loss_epoch = val;
}

uint32_t
UdpPeerNet::packets_lost()
{
    return _packets_lost;
}

uint32_t
UdpPeerNet::packet_loss()
{
    return _packet_loss;
}

uint32_t
UdpPeerNet::packet_loss_variance()
{
    return _packet_loss_variance;
}

bool
UdpPeerNet::exceeds_packet_loss_interval(uint32_t service_time)
{
    return UDP_TIME_DIFFERENCE(service_time, _packet_loss_epoch) >= PEER_PACKET_LOSS_INTERVAL;
}

uint32_t
UdpPeerNet::packets_sent()
{
    return _packets_sent;
}

void
UdpPeerNet::calculate_packet_loss(uint32_t service_time)
{
    uint32_t packet_loss = _packets_lost * PEER_PACKET_LOSS_SCALE / _packets_sent;

    _packet_loss_variance -= _packet_loss_variance / 4;

    if (packet_loss >= _packet_loss)
    {
        _packet_loss += (packet_loss - _packet_loss) / 8;
        _packet_loss_variance += (packet_loss - _packet_loss) / 4;
    }
    else
    {
        _packet_loss -= (_packet_loss - packet_loss) / 8;
        _packet_loss_variance += (_packet_loss - packet_loss) / 4;
    }

    _packet_loss_epoch = service_time;
    _packets_sent = 0;
    _packets_lost = 0;
}

void
UdpPeerNet::last_send_time(uint32_t service_time)
{
    _last_send_time = service_time;
}

void
UdpPeerNet::reset()
{
    _state = UdpPeerState::DISCONNECTED;
    _last_send_time = 0;
    _packet_throttle = PEER_DEFAULT_PACKET_THROTTLE;
    _packet_throttle_limit = PEER_PACKET_THROTTLE_SCALE;
    _packet_throttle_counter = 0;
    _packet_throttle_epoch = 0;
    _packet_throttle_acceleration = PEER_PACKET_THROTTLE_ACCELERATION;
    _packet_throttle_deceleration = PEER_PACKET_THROTTLE_DECELERATION;
    _packet_throttle_interval = PEER_PACKET_THROTTLE_INTERVAL;
    _incoming_bandwidth = 0;
    _outgoing_bandwidth = 0;
    _incoming_bandwidth_throttle_epoch = 0;
    _outgoing_bandwidth_throttle_epoch = 0;
    _packet_loss_epoch = 0;
    _packets_sent = 0;
    _packets_lost = 0;
    _packet_loss = 0;
    _packet_loss_variance = 0;
    _mtu = HOST_DEFAULT_MTU;
    _window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
}

uint32_t
UdpPeerNet::window_size()
{
    return _window_size;
}

void
UdpPeerNet::window_size(uint32_t val)
{
    _window_size = val;
}

uint32_t
UdpPeerNet::packet_throttle_interval()
{
    return _packet_throttle_interval;
}

uint32_t
UdpPeerNet::packet_throttle_acceleration()
{
    return _packet_throttle_acceleration;
}

uint32_t
UdpPeerNet::packet_throttle_deceleration()
{
    return _packet_throttle_deceleration;
}

void
UdpPeerNet::increase_packets_lost(uint32_t val)
{
    _packets_lost += val;
}

void
UdpPeerNet::increase_packets_sent(uint32_t val)
{
    _packets_sent += val;
}

void
UdpPeerNet::update_packet_throttle_counter()
{
    _packet_throttle_counter += PEER_PACKET_THROTTLE_COUNTER;
    _packet_throttle_counter %= PEER_PACKET_THROTTLE_SCALE;
}

bool
UdpPeerNet::exceeds_packet_throttle_counter()
{
    return _packet_throttle_counter > _packet_throttle;
}
