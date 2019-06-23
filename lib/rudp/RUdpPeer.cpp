#include "core/hash.h"

#include "RUdpPeer.h"

void
RUdpPeer::udp_peer_disconnect()
{
    // ...
}

std::shared_ptr<RUdpSegment>
RUdpPeer::udp_peer_receive(uint8_t &channel_id)
{
    if (_dispatched_commands.empty())
        return nullptr;

    auto incoming_command = _dispatched_commands.front();

    channel_id = incoming_command.command->header.channel_id;

    auto segment = incoming_command.segment;

    _total_waiting_data -= segment->data_length();

    return segment;
}

void
RUdpPeer::udp_peer_ping()
{
    if (!_net->state_is(RUdpPeerState::CONNECTED))
        return;

    std::shared_ptr<RUdpProtocolType> cmd = std::make_shared<RUdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_PING | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    queue_outgoing_command(cmd, nullptr, 0, 0);
}

RUdpPeer::RUdpPeer()
    : _outgoing_peer_id(0),
      _outgoing_session_id(0),
      _incoming_session_id(0),
      _last_receive_time(0),
      _ping_interval(0),
      _last_round_trip_time(0),
      _last_round_trip_time_variance(0),
      _lowest_round_trip_time(0),
      _highest_round_trip_time_variance(0),
      _needs_dispatch(false),
      _event_data(0),
      _total_waiting_data(0),
      _incoming_peer_id(0),
      _connect_id(0)
{
    _incoming_peer_id = hash32();
    _outgoing_session_id = _incoming_session_id = 0xFF;
    _data = nullptr;
}

void
RUdpPeer::queue_outgoing_command(const std::shared_ptr<RUdpProtocolType> &command,
                                 const std::shared_ptr<RUdpSegment> &segment, uint32_t offset, uint16_t length)
{
    std::shared_ptr<OutgoingCommand> outgoing_command;

    outgoing_command->command = command;
    outgoing_command->segment = segment;
    outgoing_command->fragment_offset = offset;
    outgoing_command->fragment_length = length;

    _command_pod->setup_outgoing_command(outgoing_command);
}

bool
RUdpPeer::is_disconnected()
{
    return _net->state_is(RUdpPeerState::DISCONNECTED);
}

Error
RUdpPeer::setup(const RUdpAddress &address, SysCh channel_count, uint32_t data, uint32_t in_bandwidth,
                uint32_t out_bandwidth)
{
    _channels = std::move(std::vector<std::shared_ptr<RUdpChannel>>(static_cast<int>(channel_count)));

    if (_channels.empty())
        return Error::CANT_CREATE;

    _address = address;
    _connect_id = hash32();

    _net->setup();

    std::shared_ptr<RUdpProtocolType> cmd = std::make_shared<RUdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    cmd->connect.outgoing_peer_id = htons(_incoming_peer_id);
    cmd->connect.incoming_session_id = _incoming_session_id;
    cmd->connect.outgoing_session_id = _outgoing_session_id;
    cmd->connect.mtu = htonl(_net->mtu());
    cmd->connect.window_size = htonl(_net->window_size());
    cmd->connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
    cmd->connect.incoming_bandwidth = htonl(_net->incoming_bandwidth());
    cmd->connect.outgoing_bandwidth = htonl(_net->outgoing_bandwidth());
    cmd->connect.segment_throttle_interval = htonl(_net->segment_throttle_interval());
    cmd->connect.segment_throttle_acceleration = htonl(_net->segment_throttle_acceleration());
    cmd->connect.segment_throttle_deceleration = htonl(_net->segment_throttle_deceleration());
    cmd->connect.data = data;

    queue_outgoing_command(cmd, nullptr, 0, 0);

    return Error::OK;
}

bool
RUdpPeer::needs_dispatch()
{
    return _needs_dispatch;
}

bool
RUdpPeer::needs_dispatch(bool val)
{
    _needs_dispatch = val;
}

bool
RUdpPeer::acknowledgement_exists()
{
    return !_acknowledgements.empty();
}

void
RUdpPeer::clear_acknowledgement()
{
    _acknowledgements.clear();
}

bool
RUdpPeer::dispatched_command_exists()
{
    return !_dispatched_commands.empty();
}

void
RUdpPeer::clear_dispatched_command()
{
    std::queue<IncomingCommand> empty;
    std::swap(_dispatched_commands, empty);
}

std::shared_ptr<RUdpAcknowledgement>
RUdpPeer::pop_acknowledgement()
{
    auto acknowledgement = _acknowledgements.front();

    _acknowledgements.pop_front();

    return acknowledgement;
}

const std::unique_ptr<RUdpPeerNet> &
RUdpPeer::net()
{
    return _net;
}

const std::unique_ptr<RUdpCommandPod> &
RUdpPeer::command()
{
    return _command_pod;
}

bool
RUdpPeer::state_is(RUdpPeerState state)
{
    return _net->state_is(state);
}

bool
RUdpPeer::state_is_ge(RUdpPeerState state)
{
    return _net->state_is_ge(state);
}

bool
RUdpPeer::state_is_lt(RUdpPeerState state)
{
    return _net->state_is_lt(state);
}

uint32_t
RUdpPeer::event_data()
{
    return _event_data;
}

void
RUdpPeer::event_data(uint32_t val)
{
    _event_data = val;
}

bool
RUdpPeer::load_reliable_commands_into_chamber(std::unique_ptr<RUdpChamber> &chamber, uint32_t service_time)
{
    auto can_ping = _command_pod->load_reliable_commands_into_chamber(chamber, _net, _channels, service_time);

    return can_ping;
}

bool
RUdpPeer::load_unreliable_commands_into_chamber(std::unique_ptr<RUdpChamber> &chamber)
{
    auto disconnected = _command_pod->load_unreliable_commands_into_chamber(chamber, _net);

    return disconnected;
}

uint32_t
RUdpPeer::outgoing_data_total()
{
    return _command_pod->outgoing_data_total();
}

void
RUdpPeer::outgoing_data_total(uint32_t val)
{
    _command_pod->outgoing_data_total(val);
}

uint32_t
RUdpPeer::incoming_data_total()
{
    return _command_pod->incoming_data_total();
}

void
RUdpPeer::incoming_data_total(uint32_t val)
{
    _command_pod->incoming_data_total(val);
}

uint32_t
RUdpPeer::incoming_bandwidth()
{
    return _net->incoming_bandwidth();
}

uint32_t
RUdpPeer::outgoing_bandwidth_throttle_epoch()
{
    return _net->outgoing_bandwidth_throttle_epoch();
}

void
RUdpPeer::outgoing_bandwidth_throttle_epoch(uint32_t val)
{
    _net->outgoing_bandwidth_throttle_epoch(val);
}

uint32_t
RUdpPeer::segment_throttle_limit()
{
    return _net->segment_throttle_limit();
}

void
RUdpPeer::segment_throttle_limit(uint32_t val)
{
    _net->segment_throttle_limit(val);
}

uint32_t
RUdpPeer::segment_throttle()
{
    return _net->segment_throttle();
}

void
RUdpPeer::segment_throttle(uint32_t val)
{
    _net->segment_throttle(val);
}

bool
RUdpPeer::exceeds_ping_interval(uint32_t service_time)
{
    return UDP_TIME_DIFFERENCE(service_time, _last_receive_time) >= _ping_interval;
}

bool
RUdpPeer::exceeds_mtu(size_t segment_size)
{
    return _net->mtu() - segment_size >= sizeof(RUdpProtocolPing);
}

uint16_t
RUdpPeer::outgoing_peer_id()
{
    return _outgoing_peer_id;
}

uint8_t
RUdpPeer::outgoing_session_id()
{
    return _outgoing_session_id;
}

const RUdpAddress &
RUdpPeer::address()
{
    return _address;
}

void
RUdpPeer::reset()
{
    _outgoing_peer_id = PROTOCOL_MAXIMUM_PEER_ID;
    _last_receive_time = 0;
    _ping_interval = PEER_PING_INTERVAL;
    _last_round_trip_time = PEER_DEFAULT_ROUND_TRIP_TIME;
    _lowest_round_trip_time = PEER_DEFAULT_ROUND_TRIP_TIME;
    _last_round_trip_time_variance = 0;
    _highest_round_trip_time_variance = 0;
    _event_data = 0;
    _total_waiting_data = 0;
    _connect_id = 0;

    memset(_unsequenced_window, 0, sizeof(_unsequenced_window));
}

bool
RUdpPeer::channel_exists()
{
    return !_channels.empty();
}

void
RUdpPeer::clear_channel()
{
    _channels.clear();
}
