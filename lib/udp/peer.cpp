#include "command.h"
#include "udp.h"

void
udp_peer_on_connect(const std::shared_ptr<UdpPeer> &peer)
{
    if (peer->state != UdpPeerState::CONNECTED && peer->state != UdpPeerState::DISCONNECT_LATER)
    {
        if (peer->incoming_bandwidth != 0)
            peer->host->increase_bandwidth_limited_peers();

        peer->host->increase_connected_peers();
    }
}

void
udp_peer_on_disconnect(const std::shared_ptr<UdpPeer> &peer)
{
    if (peer->state == UdpPeerState::CONNECTED || peer->state == UdpPeerState::DISCONNECT_LATER)
    {
        if (peer->incoming_bandwidth != 0)
            peer->host->decrease_bandwidth_limited_peers();

        peer->host->decrease_connected_peers();
    }
}

void
udp_peer_disconnect(std::shared_ptr<UdpPeer> &peer)
{
    // ...
}

void
udp_peer_reset_queues(const std::shared_ptr<UdpPeer> &peer)
{
    std::unique_ptr<UdpChannel> channel;

    if (peer->needs_dispatch)
        peer->needs_dispatch = false;

    if (!peer->acknowledgements.empty())
        peer->acknowledgements.clear();

    peer->sent_reliable_commands.clear();
    peer->sent_unreliable_commands.clear();
    peer->outgoing_reliable_commands.clear();
    peer->outgoing_unreliable_commands.clear();

    while (!peer->dispatched_commands.empty())
        peer->dispatched_commands.pop();

    if (!peer->channels.empty())
        peer->channels.clear();
}

void
udp_peer_reset(const std::shared_ptr<UdpPeer> &peer)
{
    udp_peer_on_disconnect(peer);

    peer->outgoing_peer_id = PROTOCOL_MAXIMUM_PEER_ID;
    peer->state = UdpPeerState::DISCONNECTED;
    peer->incoming_bandwidth = 0;
    peer->outgoing_bandwidth = 0;
    peer->incoming_bandwidth_throttle_epoch = 0;
    peer->outgoing_bandwidth_throttle_epoch = 0;
    peer->incoming_data_total = 0;
    peer->outgoing_data_total = 0;
    peer->last_send_time = 0;
    peer->last_receive_time = 0;
    peer->next_timeout = 0;
    peer->earliest_timeout = 0;
    peer->packet_loss_epoch = 0;
    peer->packets_sent = 0;
    peer->packets_lost = 0;
    peer->packet_loss = 0;
    peer->packet_loss_variance = 0;
    peer->packet_throttle = PEER_DEFAULT_PACKET_THROTTLE;
    peer->packet_throttle_limit = PEER_PACKET_THROTTLE_SCALE;
    peer->packet_throttle_counter = 0;
    peer->packet_throttle_epoch = 0;
    peer->packet_throttle_acceleration = PEER_PACKET_THROTTLE_ACCELERATION;
    peer->packet_throttle_deceleration = PEER_PACKET_THROTTLE_DECELERATION;
    peer->packet_throttle_interval = PEER_PACKET_THROTTLE_INTERVAL;
    peer->ping_interval = PEER_PING_INTERVAL;
    peer->timeout_limit = PEER_TIMEOUT_LIMIT;
    peer->timeout_minimum = PEER_TIMEOUT_MINIMUM;
    peer->timeout_maximum = PEER_TIMEOUT_MAXIMUM;
    peer->last_round_trip_time = PEER_DEFAULT_ROUND_TRIP_TIME;
    peer->lowest_round_trip_time = PEER_DEFAULT_ROUND_TRIP_TIME;
    peer->last_round_trip_time_variance = 0;
    peer->highest_round_trip_time_variance = 0;
    peer->round_trip_time = PEER_DEFAULT_ROUND_TRIP_TIME;
    peer->round_trip_time_variance = 0;
    peer->mtu = HOST_DEFAULT_MTU;
    peer->reliable_data_in_transit = 0;
    peer->outgoing_reliable_sequence_number = 0;
    peer->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    peer->incoming_unsequenced_group = 0;
    peer->outgoing_unsequenced_group = 0;
    peer->event_data = 0;
    peer->total_waiting_data = 0;
    peer->connect_id = 0;

    memset(peer->unsequenced_window, 0, sizeof(peer->unsequenced_window));

    udp_peer_reset_queues(peer);
}

std::shared_ptr<UdpPacket>
udp_peer_receive(std::shared_ptr<UdpPeer> &peer, uint8_t &channel_id)
{
    if (peer->dispatched_commands.empty())
        return nullptr;

    auto incoming_command = peer->dispatched_commands.front();

    channel_id = incoming_command.command->header.channel_id;

    auto packet = incoming_command.packet;

    peer->total_waiting_data -= packet->data_length;

    return packet;
}

void
udp_peer_ping(std::shared_ptr<UdpPeer> &peer)
{
    if (peer->state != UdpPeerState::CONNECTED)
        return;

    std::shared_ptr<UdpProtocolType> cmd = std::make_shared<UdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_PING | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    udp_peer_queue_outgoing_command(peer, cmd, nullptr, 0, 0);
}

UdpCommandPod::UdpCommandPod() :
    _incoming_data_total(0),
    _outgoing_data_total(0),
    _outgoing_reliable_sequence_number(0),
    _incoming_unsequenced_group(0),
    _outgoing_unsequenced_group(0)
{}

void
UdpCommandPod::setup_outgoing_command(UdpOutgoingCommand &outgoing_command)
{
    UdpChannel channel;

    _outgoing_data_total += udp_protocol_command_size(outgoing_command.command->header.command) + outgoing_command.fragment_length;

    if (outgoing_command.command->header.channel_id == 0xFF)
    {
        ++_outgoing_reliable_sequence_number;

        outgoing_command.reliable_sequence_number = _outgoing_reliable_sequence_number;
        outgoing_command.unreliable_sequence_number = 0;
    }
    else if (outgoing_command.command->header.command & PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
    {
        ++channel.outgoing_reliable_sequence_number;
        channel.outgoing_unreliable_seaquence_number = 0;

        outgoing_command.reliable_sequence_number = channel.outgoing_reliable_sequence_number;
        outgoing_command.unreliable_sequence_number = 0;
    }
    else if (outgoing_command.command->header.command & PROTOCOL_COMMAND_FLAG_UNSEQUENCED)
    {
        ++_outgoing_unsequenced_group;

        outgoing_command.reliable_sequence_number = 0;
        outgoing_command.unreliable_sequence_number = 0;
    }
    else
    {
        if (outgoing_command.fragment_offset == 0)
            ++channel.outgoing_unreliable_seaquence_number;

        outgoing_command.reliable_sequence_number = channel.outgoing_reliable_sequence_number;
        outgoing_command.unreliable_sequence_number = channel.outgoing_unreliable_seaquence_number;
    }

    outgoing_command.send_attempts = 0;
    outgoing_command.sent_time = 0;
    outgoing_command.round_trip_timeout = 0;
    outgoing_command.round_trip_timeout_limit = 0;
    outgoing_command.command->header.reliable_sequence_number = htons(outgoing_command.reliable_sequence_number);

    auto cmd = outgoing_command.command->header.command & PROTOCOL_COMMAND_MASK;

    if (cmd == PROTOCOL_COMMAND_SEND_UNRELIABLE)
    {
        outgoing_command.command->send_unreliable.unreliable_sequence_number = htons(outgoing_command.unreliable_sequence_number);
    }
    else if (cmd == PROTOCOL_COMMAND_SEND_UNSEQUENCED)
    {
        outgoing_command.command->send_unsequenced.unsequenced_group = htons(_outgoing_unsequenced_group);
    }

    if (outgoing_command.command->header.command & PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
        _outgoing_reliable_commands.push_back(outgoing_command);
    else
        _outgoing_unreliable_commands.push_back(outgoing_command);
}

UdpPeer::UdpPeer() : _outgoing_peer_id(0),
                     _outgoing_session_id(0),
                     _incoming_session_id(0),
                     _state(UdpPeerState::DISCONNECTED),
                     _incoming_bandwidth(0),
                     _outgoing_bandwidth(0),
                     _incoming_bandwidth_throttle_epoch(0),
                     _outgoing_bandwidth_throttle_epoch(0),
                     _last_send_time(0),
                     _last_receive_time(0),
                     _next_timeout(0),
                     _earliest_timeout(0),
                     _packet_loss_epoch(0),
                     _packets_sent(0),
                     _packets_lost(0),
                     _packet_loss(0),
                     _packet_loss_variance(0),
                     _packet_throttle(0),
                     _packet_throttle_limit(0),
                     _packet_throttle_counter(0),
                     _packet_throttle_epoch(0),
                     _packet_throttle_acceleration(0),
                     _packet_throttle_deceleration(0),
                     _packet_throttle_interval(0),
                     _ping_interval(0),
                     _timeout_limit(0),
                     _timeout_minimum(0),
                     _timeout_maximum(0),
                     _last_round_trip_time(0),
                     _last_round_trip_time_variance(0),
                     _lowest_round_trip_time(0),
                     _highest_round_trip_time_variance(0),
                     _round_trip_time(0),
                     _round_trip_time_variance(0),
                     _mtu(0),
                     _window_size(0),
                     _reliable_data_in_transit(0),
                     _needs_dispatch(false),
                     _event_data(0),
                     _total_waiting_data(0),
                     _incoming_peer_id(0),
                     _connect_id(0)
{
    _data = nullptr;
}

UdpOutgoingCommand
UdpPeer::queue_outgoing_command(const std::shared_ptr<UdpProtocolType> &command, const std::shared_ptr<UdpPacket> &packet, uint32_t offset, uint16_t length)
{
    UdpOutgoingCommand outgoing_command;

    outgoing_command.command = command;
    outgoing_command.packet = packet;
    outgoing_command.fragment_offset = offset;
    outgoing_command.fragment_length = length;

    _command_pod->setup_outgoing_command(outgoing_command);

    return outgoing_command;
}
