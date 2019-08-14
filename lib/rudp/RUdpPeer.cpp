#include "core/hash.h"

#include "RUdpCommon.h"
#include "RUdpPeer.h"

RUdpPeer::RUdpPeer()
    : command_pod_(std::make_unique<RUdpCommandPod>()),
      connect_id_(),
      data_(nullptr),
      event_data_(),
      highest_round_trip_time_variance_(),
      incoming_peer_id_(),
      incoming_session_id_(0xFF),
      last_receive_time_(),
      last_round_trip_time_(PEER_DEFAULT_ROUND_TRIP_TIME),
      last_round_trip_time_variance_(),
      lowest_round_trip_time_(PEER_DEFAULT_ROUND_TRIP_TIME),
      needs_dispatch_(),
      net_(std::make_unique<RUdpPeerNet>()),
      outgoing_peer_id_(PROTOCOL_MAXIMUM_PEER_ID),
      outgoing_session_id_(0xFF),
      ping_interval_(PEER_PING_INTERVAL),
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
    //channels_ = std::move(std::vector<std::shared_ptr<RUdpChannel>>(static_cast<int>(channel_count)));
    for (auto i = 0; i < static_cast<int>(channel_count); ++i)
        channels_.emplace_back(std::make_shared<RUdpChannel>());

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

    //std::shared_ptr<RUdpSegment> seg = std::make_shared<RUdpSegment>();

    QueueOutgoingCommand(protocol_type, nullptr, 0, 0);

    return Error::OK;
}

void
RUdpPeer::SetupConnectedPeer(const RUdpProtocolType *cmd,
                             const RUdpAddress &received_address,
                             uint32_t host_incoming_bandwidth,
                             uint32_t host_outgoing_bandwidth,
                             uint32_t channel_count)
{
    net_->state(RUdpPeerState::ACKNOWLEDGING_CONNECT);
    connect_id_ = cmd->connect.connect_id;
    address_ = received_address;
    outgoing_peer_id_ = ntohs(cmd->connect.outgoing_peer_id);
    net_->incoming_bandwidth(ntohl(cmd->connect.incoming_bandwidth));
    net_->outgoing_bandwidth(ntohl(cmd->connect.outgoing_bandwidth));
    net_->segment_throttle_interval(ntohl(cmd->connect.segment_throttle_interval));
    net_->segment_throttle_acceleration(ntohl(cmd->connect.segment_throttle_acceleration));
    net_->segment_throttle_deceleration(ntohl(cmd->connect.segment_throttle_deceleration));
    event_data_ = ntohl(cmd->connect.data);

    auto incoming_session_id = cmd->connect.incoming_session_id == 0xFF ? outgoing_session_id_ :
                               cmd->connect.incoming_session_id;
    incoming_session_id = (incoming_session_id + 1) & (PROTOCOL_HEADER_SESSION_MASK >> PROTOCOL_HEADER_SESSION_SHIFT);

    if (incoming_session_id == outgoing_session_id_)
        incoming_session_id = (incoming_session_id + 1) &
            (PROTOCOL_HEADER_SESSION_MASK >> PROTOCOL_HEADER_SESSION_SHIFT);

    outgoing_session_id_ = incoming_session_id;

    auto outgoing_session_id = cmd->connect.outgoing_session_id == 0xFF ? incoming_session_id_ :
                               cmd->connect.outgoing_session_id;
    outgoing_session_id = (outgoing_session_id + 1) & (PROTOCOL_HEADER_SESSION_MASK >> PROTOCOL_HEADER_SESSION_SHIFT);

    if (outgoing_session_id == incoming_session_id)
        outgoing_session_id =
            (outgoing_session_id + 1) & (PROTOCOL_HEADER_SESSION_MASK >> PROTOCOL_HEADER_SESSION_SHIFT);

    incoming_session_id_ = outgoing_session_id;

    for (auto i = 0; i < static_cast<int>(channel_count); ++i)
        channels_.emplace_back(std::make_shared<RUdpChannel>());

    for (auto &ch : channels_)
        ch->Reset();

    auto mtu = ntohl(cmd->connect.mtu);

    if (mtu < PROTOCOL_MINIMUM_MTU)
    {
        mtu = PROTOCOL_MINIMUM_MTU;
    }
    else if (mtu > PROTOCOL_MAXIMUM_MTU)
    {
        mtu = PROTOCOL_MAXIMUM_MTU;
    }

    net_->mtu(mtu);

    if (host_outgoing_bandwidth == 0 && net_->incoming_bandwidth() == 0)
    {
        net_->window_size(PROTOCOL_MAXIMUM_WINDOW_SIZE);
    }
    else if (host_outgoing_bandwidth == 0 || net_->incoming_bandwidth() == 0)
    {
        auto ws = (
            std::max(host_outgoing_bandwidth, net_->incoming_bandwidth()) / PEER_WINDOW_SIZE_SCALE
        ) * PROTOCOL_MINIMUM_WINDOW_SIZE;

        net_->window_size(ws);
    }
    else
    {
        auto ws = (
            std::min(host_outgoing_bandwidth, net_->incoming_bandwidth()) / PEER_WINDOW_SIZE_SCALE
        ) * PROTOCOL_MINIMUM_WINDOW_SIZE;

        net_->window_size(ws);
    }

    if (net_->window_size() < PROTOCOL_MINIMUM_WINDOW_SIZE)
    {
        net_->window_size(PROTOCOL_MINIMUM_WINDOW_SIZE);
    }
    else if (net_->window_size() > PROTOCOL_MAXIMUM_WINDOW_SIZE)
    {
        net_->window_size(PROTOCOL_MAXIMUM_WINDOW_SIZE);
    }

    auto window_size = 0;

    if (host_incoming_bandwidth == 0)
    {
        window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    }
    else
    {
        window_size = (host_incoming_bandwidth / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;
    }

    if (window_size > ntohl(cmd->connect.window_size))
        window_size = ntohl(cmd->connect.window_size);

    if (window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
    {
        window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;
    }
    else if (window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
    {
        window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    }

    std::shared_ptr<RUdpProtocolType> verify_cmd = std::make_shared<RUdpProtocolType>();

    verify_cmd->header.command = PROTOCOL_COMMAND_VERIFY_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    verify_cmd->header.channel_id = 0xFF;
    verify_cmd->verify_connect.outgoing_peer_id = htons(incoming_peer_id_);
    verify_cmd->verify_connect.incoming_session_id = incoming_session_id;
    verify_cmd->verify_connect.outgoing_session_id = outgoing_session_id;
    verify_cmd->verify_connect.mtu = htonl(net_->mtu());
    verify_cmd->verify_connect.window_size = htonl(window_size);
    verify_cmd->verify_connect.channel_count = htonl(channel_count);
    verify_cmd->verify_connect.incoming_bandwidth = htonl(host_incoming_bandwidth);
    verify_cmd->verify_connect.outgoing_bandwidth = htonl(host_outgoing_bandwidth);
    verify_cmd->verify_connect.segment_throttle_interval = htonl(net_->segment_throttle_interval());
    verify_cmd->verify_connect.segment_throttle_acceleration = htonl(net_->segment_throttle_acceleration());
    verify_cmd->verify_connect.segment_throttle_deceleration = htonl(net_->segment_throttle_deceleration());
    verify_cmd->verify_connect.connect_id = connect_id_;

    QueueOutgoingCommand(verify_cmd, nullptr, 0, 0);
}

void
RUdpPeer::Ping()
{
    if (!net_->StateIs(RUdpPeerState::CONNECTED))
        return;

    std::shared_ptr<RUdpProtocolType> cmd = std::make_shared<RUdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_PING | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    //std::shared_ptr<RUdpSegment> seg = std::make_shared<RUdpSegment>();

    QueueOutgoingCommand(cmd, nullptr, 0, 0);
}

void
RUdpPeer::QueueAcknowledgement(const RUdpProtocolType *cmd, uint16_t sent_time)
{
    if (cmd->header.channel_id < channels_.size())
    {
        auto channel = channels_.at(cmd->header.channel_id);
        auto reliable_window = cmd->header.reliable_sequence_number / PEER_RELIABLE_WINDOW_SIZE;
        auto current_window = channel->incoming_reliable_sequence_number / PEER_FREE_RELIABLE_WINDOWS;

        if (cmd->header.reliable_sequence_number < channel->incoming_reliable_sequence_number)
            reliable_window += PEER_RELIABLE_WINDOWS;

        if (reliable_window >= current_window + PEER_FREE_RELIABLE_WINDOWS - 1 && reliable_window <= current_window + PEER_FREE_RELIABLE_WINDOWS)
            return;

        auto ack = std::make_shared<RUdpAcknowledgement>();
        if (ack == nullptr)
            return;

        outgoing_data_total(sizeof(RUdpProtocolAcknowledge));

        ack->sent_time = sent_time;
        ack->command = *cmd;

        acknowledgements_.push_back(ack);
    }
}

// TODO: Is segment necessary as an argument?
void
RUdpPeer::QueueOutgoingCommand(const std::shared_ptr<RUdpProtocolType> &protocol_type,
                               const std::shared_ptr<RUdpSegment> &segment, uint32_t offset, uint16_t length)
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

    total_waiting_data_ -= segment->Size();

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
RUdpPeer::EventOccur(const RUdpAddress &address, uint8_t session_id)
{
    if (net_->StateIs(RUdpPeerState::DISCONNECTED))
        return false;

    if (net_->StateIs(RUdpPeerState::ZOMBIE))
        return false;

    if (address_ != address)
        return false;

    if (outgoing_peer_id_ < PROTOCOL_MAXIMUM_PEER_ID && session_id != incoming_session_id_)
        return false;

    return true;
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
    this->Reset(incoming_peer_id_);
}

void
RUdpPeer::Reset(uint16_t peer_idx)
{
    acknowledgements_.clear();
    channels_.clear();
    command_pod_->Reset();

    std::queue<IncomingCommand> empty;
    dispatched_commands_.swap(empty);

    address_.Reset();

    connect_id_ = 0;
    data_ = nullptr;
    event_data_ = 0;
    highest_round_trip_time_variance_ = 0;
    incoming_peer_id_ = peer_idx;
    incoming_session_id_ = 0xFF;
    last_receive_time_ = 0;
    last_round_trip_time_ = PEER_DEFAULT_ROUND_TRIP_TIME;
    last_round_trip_time_variance_ = 0;
    lowest_round_trip_time_ = PEER_DEFAULT_ROUND_TRIP_TIME;
    needs_dispatch_ = false;
    outgoing_peer_id_ = PROTOCOL_MAXIMUM_PEER_ID;
    outgoing_session_id_ = 0xFF;
    ping_interval_ = PEER_PING_INTERVAL;
    total_waiting_data_ = 0;

    memset(&unsequenced_windows_, 0, unsequenced_windows_.size());
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
