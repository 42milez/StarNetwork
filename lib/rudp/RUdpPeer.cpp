#include "core/hash.h"

#include "RUdpPeer.h"

RUdpPeer::RUdpPeer()
    : command_pod_(std::make_unique<RUdpCommandPod>()),
      connect_id_(),
      data_(nullptr),
      event_data_(),
      highest_round_trip_time_variance_(),
      incoming_peer_id_(hash32()),
      incoming_session_id_(0xFF),
      last_receive_time_(),
      last_round_trip_time_(),
      last_round_trip_time_variance_(),
      lowest_round_trip_time_(),
      needs_dispatch_(false),
      net_(std::make_unique<RUdpPeerNet>()),
      outgoing_peer_id_(),
      outgoing_session_id_(0xFF),
      ping_interval_(),
      total_waiting_data_(),
      unsequenced_windows_()
{}

void
RUdpPeer::Disconnect()
{
    // ...
}

Error
RUdpPeer::Setup(const RUdpAddress &address, SysCh channel_count, uint32_t in_bandwidth,
                uint32_t out_bandwidth, uint32_t data)
{
    channels_ = std::move(std::vector<std::shared_ptr<RUdpChannel>>(static_cast<int>(channel_count)));

    if (channels_.empty())
        return Error::CANT_CREATE;

    address_ = address;
    connect_id_ = hash32();

    net_->setup();

    std::shared_ptr<RUdpProtocolType> protocol_type = std::make_shared<RUdpProtocolType>();

    protocol_type->header.command = PROTOCOL_COMMAND_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    protocol_type->header.channel_id = 0xFF;

    protocol_type->connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
    protocol_type->connect.connect_id = connect_id_;
    protocol_type->connect.data = htonl(data);
    protocol_type->connect.mtu = htonl(net_->mtu());
    protocol_type->connect.window_size = htonl(net_->window_size());

    protocol_type->connect.incoming_bandwidth = htonl(net_->incoming_bandwidth());
    protocol_type->connect.incoming_session_id = incoming_session_id_;

    protocol_type->connect.outgoing_bandwidth = htonl(net_->outgoing_bandwidth());
    protocol_type->connect.outgoing_peer_id = htons(incoming_peer_id_);
    protocol_type->connect.outgoing_session_id = outgoing_session_id_;

    protocol_type->connect.segment_throttle_acceleration = htonl(net_->segment_throttle_acceleration());
    protocol_type->connect.segment_throttle_deceleration = htonl(net_->segment_throttle_deceleration());
    protocol_type->connect.segment_throttle_interval = htonl(net_->segment_throttle_interval());

    std::shared_ptr<RUdpSegment> seg = std::make_shared<RUdpSegment>();

    QueueOutgoingCommand(protocol_type, seg, 0, 0);

    return Error::OK;
}

void
RUdpPeer::Ping()
{
    if (!net_->StateIs(RUdpPeerState::CONNECTED))
        return;

    std::shared_ptr<RUdpProtocolType> cmd = std::make_shared<RUdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_PING | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    std::shared_ptr<RUdpSegment> seg = std::make_shared<RUdpSegment>();

    QueueOutgoingCommand(cmd, seg, 0, 0);
}

// TODO: Is segment necessary as an argument?
void
RUdpPeer::QueueOutgoingCommand(std::shared_ptr<RUdpProtocolType> &protocol_type,
                               std::shared_ptr<RUdpSegment> &segment, uint32_t offset, uint16_t length)
{
    std::shared_ptr<OutgoingCommand> outgoing_command = std::make_shared<OutgoingCommand>();

    outgoing_command->protocol_type = protocol_type;
    outgoing_command->segment = segment;
    outgoing_command->fragment_length = length;
    outgoing_command->fragment_offset = offset;

    auto channel_id = protocol_type->header.channel_id;

    if (channel_id < channels_.size())
        command_pod_->setup_outgoing_command(outgoing_command, channels_.at(channel_id));
    else
        command_pod_->setup_outgoing_command(outgoing_command, nullptr);
}

std::shared_ptr<RUdpSegment>
RUdpPeer::Receive(uint8_t &channel_id)
{
    if (dispatched_commands_.empty())
        return nullptr;

    auto incoming_command = dispatched_commands_.front();

    channel_id = incoming_command.command->header.channel_id;

    auto segment = incoming_command.segment;

    total_waiting_data_ -= segment->data_length();

    return segment;
}

bool
RUdpPeer::AcknowledgementExists()
{
    return !acknowledgements_.empty();
}

void
RUdpPeer::ClearAcknowledgement()
{
    acknowledgements_.clear();
}

std::shared_ptr<RUdpAcknowledgement>
RUdpPeer::PopAcknowledgement()
{
    auto acknowledgement = acknowledgements_.front();

    acknowledgements_.pop_front();

    return acknowledgement;
}

bool
RUdpPeer::ChannelExists()
{
    return !channels_.empty();
}

void
RUdpPeer::ClearChannel()
{
    channels_.clear();
}

void
RUdpPeer::ClearDispatchedCommandQueue()
{
    std::queue<IncomingCommand> empty;
    std::swap(dispatched_commands_, empty);
}

bool
RUdpPeer::DispatchedCommandExists()
{
    return !dispatched_commands_.empty();
}

bool
RUdpPeer::Disconnected()
{
    return net_->StateIs(RUdpPeerState::DISCONNECTED);
}

bool
RUdpPeer::LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, uint32_t service_time)
{
    auto can_ping = command_pod_->LoadReliableCommandsIntoChamber(chamber, net_, channels_, service_time);

    return can_ping;
}

bool
RUdpPeer::LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber)
{
    auto disconnected = command_pod_->LoadUnreliableCommandsIntoChamber(chamber, net_);

    return disconnected;
}

void
RUdpPeer::Reset()
{
    outgoing_peer_id_ = PROTOCOL_MAXIMUM_PEER_ID;
    last_receive_time_ = 0;
    ping_interval_ = PEER_PING_INTERVAL;
    last_round_trip_time_ = PEER_DEFAULT_ROUND_TRIP_TIME;
    lowest_round_trip_time_ = PEER_DEFAULT_ROUND_TRIP_TIME;
    last_round_trip_time_variance_ = 0;
    highest_round_trip_time_variance_ = 0;
    event_data_ = 0;
    total_waiting_data_ = 0;
    connect_id_ = 0;

    memset(unsequenced_windows_, 0, sizeof(unsequenced_windows_));
}

bool
RUdpPeer::StateIs(RUdpPeerState state)
{
    return net_->StateIs(state);
}

bool
RUdpPeer::StateIsGreaterThanOrEqual(RUdpPeerState state)
{
    return net_->StateIsGreaterThanOrEqual(state);
}

bool
RUdpPeer::StateIsLessThanOrEqual(RUdpPeerState state)
{
    return net_->StateIsLessThanOrEqual(state);
}

bool
RUdpPeer::ExceedsMTU(size_t segment_size)
{
    return net_->mtu() - segment_size >= sizeof(RUdpProtocolPing);
}

bool
RUdpPeer::ExceedsPingInterval(uint32_t service_time)
{
    return UDP_TIME_DIFFERENCE(service_time, last_receive_time_) >= ping_interval_;
}
