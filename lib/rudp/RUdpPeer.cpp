#include "core/hash.h"

#include "RUdpPeer.h"

RUdpPeer::RUdpPeer()
    : _connect_id(),
      _data(nullptr),
      _event_data(),
      _highest_round_trip_time_variance(),
      _incoming_peer_id(hash32()),
      _incoming_session_id(0xFF),
      _last_receive_time(),
      _last_round_trip_time(),
      _last_round_trip_time_variance(),
      _lowest_round_trip_time(),
      _needs_dispatch(false),
      _outgoing_peer_id(),
      _outgoing_session_id(0xFF),
      _ping_interval(),
      _total_waiting_data(),
      _unsequenced_window()
{}

void
RUdpPeer::Disconnect()
{
    // ...
}

Error
RUdpPeer::Setup(const RUdpAddress &address, SysCh channel_count, uint32_t data, uint32_t in_bandwidth,
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

    QueueOutgoingCommand(cmd, nullptr, 0, 0);

    return Error::OK;
}

void
RUdpPeer::Ping()
{
    if (!_net->StateIs(RUdpPeerState::CONNECTED))
        return;

    std::shared_ptr<RUdpProtocolType> cmd = std::make_shared<RUdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_PING | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    QueueOutgoingCommand(cmd, nullptr, 0, 0);
}

void
RUdpPeer::QueueOutgoingCommand(const std::shared_ptr<RUdpProtocolType> &command,
                               const std::shared_ptr<RUdpSegment> &segment, uint32_t offset, uint16_t length)
{
    std::shared_ptr<OutgoingCommand> outgoing_command;

    outgoing_command->command = command;
    outgoing_command->segment = segment;
    outgoing_command->fragment_offset = offset;
    outgoing_command->fragment_length = length;

    _command_pod->setup_outgoing_command(outgoing_command);
}

std::shared_ptr<RUdpSegment>
RUdpPeer::Receive(uint8_t &channel_id)
{
    if (_dispatched_commands.empty())
        return nullptr;

    auto incoming_command = _dispatched_commands.front();

    channel_id = incoming_command.command->header.channel_id;

    auto segment = incoming_command.segment;

    _total_waiting_data -= segment->data_length();

    return segment;
}

bool
RUdpPeer::AcknowledgementExists()
{
    return !_acknowledgements.empty();
}

void
RUdpPeer::ClearAcknowledgement()
{
    _acknowledgements.clear();
}

std::shared_ptr<RUdpAcknowledgement>
RUdpPeer::PopAcknowledgement()
{
    auto acknowledgement = _acknowledgements.front();

    _acknowledgements.pop_front();

    return acknowledgement;
}

bool
RUdpPeer::ChannelExists()
{
    return !_channels.empty();
}

void
RUdpPeer::ClearChannel()
{
    _channels.clear();
}

void
RUdpPeer::ClearDispatchedCommandQueue()
{
    std::queue<IncomingCommand> empty;
    std::swap(_dispatched_commands, empty);
}

bool
RUdpPeer::DispatchedCommandExists()
{
    return !_dispatched_commands.empty();
}

bool
RUdpPeer::Disconnected()
{
    return _net->StateIs(RUdpPeerState::DISCONNECTED);
}

bool
RUdpPeer::LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, uint32_t service_time)
{
    auto can_ping = _command_pod->LoadReliableCommandsIntoChamber(chamber, _net, _channels, service_time);

    return can_ping;
}

bool
RUdpPeer::LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber)
{
    auto disconnected = _command_pod->LoadUnreliableCommandsIntoChamber(chamber, _net);

    return disconnected;
}

void
RUdpPeer::Reset()
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
RUdpPeer::StateIs(RUdpPeerState state)
{
    return _net->StateIs(state);
}

bool
RUdpPeer::StateIsGreaterThanOrEqual(RUdpPeerState state)
{
    return _net->StateIsGreaterThanOrEqual(state);
}

bool
RUdpPeer::StateIsLessThanOrEqual(RUdpPeerState state)
{
    return _net->StateIsLessThanOrEqual(state);
}

bool
RUdpPeer::ExceedsMTU(size_t segment_size)
{
    return _net->mtu() - segment_size >= sizeof(RUdpProtocolPing);
}

bool
RUdpPeer::ExceedsPingInterval(uint32_t service_time)
{
    return UDP_TIME_DIFFERENCE(service_time, _last_receive_time) >= _ping_interval;
}
