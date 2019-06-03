#include "command.h"
#include "udp.h"

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

std::shared_ptr<UdpPeer>
UdpPeerPod::available_peer_exists()
{
    for (auto &peer : _peers)
    {
        if (peer->is_disconnected())
            return peer;
    }

    return nullptr;
}

void
UdpPeerPod::bandwidth_throttle(uint32_t _incoming_bandwidth, uint32_t _outgoing_bandwidth, bool &_recalculate_bandwidth_limits)
{
    auto time_current = udp_time_get();
    auto time_elapsed = time_current - _bandwidth_throttle_epoch;
    auto peers_remaining = _connected_peers;
    auto data_total = ~0u;
    auto bandwidth = ~0u;
    auto throttle = 0;
    auto bandwidth_limit = 0;
    auto needs_adjustment = _bandwidth_limited_peers > 0 ? true : false;

    if (time_elapsed < HOST_BANDWIDTH_THROTTLE_INTERVAL)
        return;

    _bandwidth_throttle_epoch = time_current;

    if (peers_remaining == 0)
        return;

    //  Throttle outgoing bandwidth
    // --------------------------------------------------

    if (_outgoing_bandwidth != 0)
    {
        data_total = 0;
        bandwidth = _outgoing_bandwidth * (time_elapsed / 1000);

        for (const auto &peer : _peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            data_total += peer->outgoing_data_total;
        }
    }

    //  Throttle peer bandwidth : Case A ( adjustment is needed )
    // --------------------------------------------------

    while (peers_remaining > 0 && needs_adjustment)
    {
        needs_adjustment = false;

        if (data_total <= bandwidth)
            throttle = PEER_PACKET_THROTTLE_SCALE;
        else
            throttle = (bandwidth * PEER_PACKET_THROTTLE_SCALE) / data_total;

        for (auto &peer : _peers)
        {
            uint32_t peer_bandwidth;

            if ((IS_PEER_CONNECTED(peer)) ||
                peer->incoming_bandwidth == 0 ||
                peer->outgoing_bandwidth_throttle_epoch == time_current)
            {
                continue;
            }

            peer_bandwidth = peer->incoming_bandwidth * (time_elapsed / 1000);
            if ((throttle * peer->outgoing_data_total) / PEER_PACKET_THROTTLE_SCALE <= peer_bandwidth)
                continue;

            peer->packet_throttle_limit = (peer_bandwidth * PEER_PACKET_THROTTLE_SCALE) / peer->outgoing_data_total;

            if (peer->packet_throttle_limit == 0)
                peer->packet_throttle_limit = 1;

            if (peer->packet_throttle > peer->packet_throttle_limit)
                peer->packet_throttle = peer->packet_throttle_limit;

            peer->outgoing_bandwidth_throttle_epoch = time_current;
            peer->incoming_data_total = 0;
            peer->outgoing_data_total = 0;

            needs_adjustment = true;

            --peers_remaining;

            bandwidth -= peer_bandwidth;
            data_total -= peer_bandwidth;
        }
    }

    //  Throttle peer bandwidth : Case B ( adjustment is NOT needed )
    // --------------------------------------------------

    if (peers_remaining > 0)
    {
        if (data_total <= bandwidth)
            throttle = PEER_PACKET_THROTTLE_SCALE;
        else
            throttle = (bandwidth * PEER_PACKET_THROTTLE_SCALE) / data_total;

        for (auto &peer : _peers)
        {
            if ((IS_PEER_CONNECTED(peer)) || peer->outgoing_bandwidth_throttle_epoch == time_current)
                continue;

            peer->packet_throttle_limit = throttle;

            if (peer->packet_throttle > peer->packet_throttle_limit)
                peer->packet_throttle = peer->packet_throttle_limit;

            peer->incoming_data_total = 0;
            peer->outgoing_data_total = 0;
        }
    }

    //  Recalculate Bandwidth Limits
    // --------------------------------------------------

    if (_recalculate_bandwidth_limits)
    {
        _recalculate_bandwidth_limits = false;
        peers_remaining = _connected_peers;
        bandwidth = _incoming_bandwidth;
        needs_adjustment = true;

        if (bandwidth == 0)
        {
            bandwidth_limit = 0;
        }
        else
        {
            while (peers_remaining > 0 && needs_adjustment)
            {
                needs_adjustment = false;
                bandwidth_limit = bandwidth / peers_remaining;

                for (auto &peer: _peers)
                {
                    if ((IS_PEER_CONNECTED(peer)) || peer->incoming_bandwidth_throttle_epoch == time_current)
                        continue;

                    if (peer->outgoing_bandwidth > 0 && peer->outgoing_bandwidth >= bandwidth_limit)
                        continue;

                    peer->incoming_bandwidth_throttle_epoch = time_current;

                    needs_adjustment = true;

                    --peers_remaining;

                    bandwidth -= peer->outgoing_bandwidth;
                }
            }
        }

        std::shared_ptr<UdpProtocolType> cmd;

        for (auto &peer : _peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            cmd->header.command = PROTOCOL_COMMAND_BANDWIDTH_LIMIT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
            cmd->header.channel_id = 0xFF;
            cmd->bandwidth_limit.outgoing_bandwidth = htonl(_outgoing_bandwidth);

            if (peer->incoming_bandwidth_throttle_epoch == time_current)
                cmd->bandwidth_limit.incoming_bandwidth = htonl(peer->outgoing_bandwidth);
            else
                cmd->bandwidth_limit.incoming_bandwidth = htonl(bandwidth_limit);

            udp_peer_queue_outgoing_command(peer, cmd, nullptr, 0, 0);
        }
    }
}

UdpPeerPod::UdpPeerPod(size_t peer_count) :
    _bandwidth_limited_peers(0),
    _bandwidth_throttle_epoch(0),
    _connected_peers(0),
    _peers(peer_count),
    _peer_count(peer_count)
{}

int
UdpPeerPod::send_outgoing_commands(std::unique_ptr<UdpEvent> &event, bool check_for_timeouts)
{
    uint8_t header_data[sizeof(UdpProtocolHeader) + sizeof(uint32_t)];
    auto *header = reinterpret_cast<UdpProtocolHeader *>(header_data);

    _continue_sending = true;

    while (_continue_sending)
    {
        _continue_sending = false;

        for (auto &peer : _peers)
        {
            if (peer->state == UdpPeerState::DISCONNECTED || peer->state == UdpPeerState::ZOMBIE)
                continue;

            _header_flags = 0;
            _command_count = 0;
            _buffer_count = 1;
            _packet_size = sizeof(UdpProtocolHeader);

            //  ACKを返す
            // --------------------------------------------------

            if (!peer->acknowledgements.empty())
                _udp_protocol_send_acknowledgements(peer);

            //  タイムアウト処理
            // --------------------------------------------------

            if (check_for_timeouts &&
                !peer->sent_reliable_commands.empty() &&
                UDP_TIME_GREATER_EQUAL(_service_time, peer->next_timeout) &&
                _udp_protocol_check_timeouts(peer, event) == 1)
            {
                if (event->type != UdpEventType::NONE)
                    return 1;
                else
                    continue;
            }

            //  送信バッファに Reliable Command を転送する
            // --------------------------------------------------

            if ((!peer->outgoing_reliable_commands.empty() || _udp_protocol_send_reliable_outgoing_commands(peer)) &&
                peer->sent_reliable_commands.empty() &&
                UDP_TIME_DIFFERENCE(_service_time, peer->last_receive_time) >= peer->ping_interval &&
                peer->mtu - _packet_size >= sizeof(UdpProtocolPing))
            {
                udp_peer_ping(peer);

                // ping コマンドをバッファに転送
                _udp_protocol_send_reliable_outgoing_commands(peer);
            }

            //  送信バッファに Unreliable Command を転送する
            // --------------------------------------------------

            if (!peer->outgoing_unreliable_commands.empty())
                _udp_protocol_send_unreliable_outgoing_commands(peer);

            if (_command_count == 0)
                continue;

            if (peer->packet_loss_epoch == 0)
            {
                peer->packet_loss_epoch = _service_time;
            }
            else if (UDP_TIME_DIFFERENCE(_service_time, peer->packet_loss_epoch) >= PEER_PACKET_LOSS_INTERVAL &&
                     peer->packets_sent > 0)
            {
                uint32_t packet_loss = peer->packets_lost * PEER_PACKET_LOSS_SCALE / peer->packets_sent;

                peer->packet_loss_variance -= peer->packet_loss_variance / 4;

                if (packet_loss >= peer->packet_loss)
                {
                    peer->packet_loss += (packet_loss - peer->packet_loss) / 8;
                    peer->packet_loss_variance += (packet_loss - peer->packet_loss) / 4;
                }
                else
                {
                    peer->packet_loss -= (peer->packet_loss - packet_loss) / 8;
                    peer->packet_loss_variance += (peer->packet_loss - packet_loss) / 4;
                }

                peer->packet_loss_epoch = _service_time;
                peer->packets_sent = 0;
                peer->packets_lost = 0;
            }

            if (_header_flags & PROTOCOL_HEADER_FLAG_SENT_TIME)
            {
                header->sent_time = htons(_service_time & 0xFFFF);
                _buffers[0].data_length = sizeof(UdpProtocolHeader);
            }
            else
            {
                _buffers[0].data_length = (size_t) &((UdpProtocolHeader *) 0)->sent_time; // ???
            }

            auto should_compress = false;

            if (_compressor->compress != nullptr)
            {
                // ...
            }

            if (peer->outgoing_peer_id < PROTOCOL_MAXIMUM_PEER_ID)
                _header_flags |= peer->outgoing_session_id << PROTOCOL_HEADER_SESSION_SHIFT;

            header->peer_id = htons(peer->outgoing_peer_id | _header_flags);

            if (_checksum != nullptr)
            {
                // ...
            }

            if (should_compress)
            {
                // ...
            }

            peer->last_send_time = _service_time;

            auto sent_length = _udp_socket_send(peer->address);

            _udp_protocol_remove_sent_unreliable_commands(peer);

            if (sent_length < 0)
                return -1;

            _total_sent_data += sent_length;

            ++_total_sent_packets;
        }
    }

    return 0;
}

int
UdpPeerPod::dispatch_incoming_commands(std::unique_ptr<UdpEvent> &event, bool &_recalculate_bandwidth_limits)
{
    while (!_dispatch_queue.empty())
    {
        auto peer = _pop_peer_from_dispatch_queue();

        peer->needs_dispatch = false;

        if (peer->state == UdpPeerState::CONNECTION_PENDING ||
            peer->state == UdpPeerState::CONNECTION_SUCCEEDED)
        {
            // ピアが接続したら接続中ピアのカウンタを増やし、切断したら減らす
            _udp_protocol_change_state(peer, UdpPeerState::CONNECTED);

            event->type = UdpEventType::CONNECT;
            event->peer = peer;
            event->data = peer->event_data;

            return 1;
        }
        else if (peer->state == UdpPeerState::ZOMBIE)
        {
            _recalculate_bandwidth_limits = true;

            event->type = UdpEventType::DISCONNECT;
            event->peer = peer;
            event->data = peer->event_data;

            // ゾンビ状態になったピアはリセットする
            udp_peer_reset(peer);

            return 1;
        }
        else if (peer->state == UdpPeerState::CONNECTED)
        {
            if (peer->dispatched_commands.empty())
                continue;

            // 接続済みのピアからはコマンドを受信する
            event->packet = udp_peer_receive(peer, event->channel_id);

            if (event->packet == nullptr)
                continue;

            event->type = UdpEventType::RECEIVE;
            event->peer = peer;

            // ディスパッチすべきピアが残っている場合は、ディスパッチ待ちキューにピアを投入する
            if (!peer->dispatched_commands.empty())
            {
                peer->needs_dispatch = true;

                _dispatch_queue.push(peer);
            }

            return 1;
        }
    }

    return 0;
}

void
UdpPeer::udp_peer_on_connect()
{
    if (peer->state != UdpPeerState::CONNECTED && peer->state != UdpPeerState::DISCONNECT_LATER)
    {
        if (peer->incoming_bandwidth != 0)
            peer->host->increase_bandwidth_limited_peers();

        peer->host->increase_connected_peers();
    }
}

void
UdpPeer::udp_peer_on_disconnect()
{
    if (peer->state == UdpPeerState::CONNECTED || peer->state == UdpPeerState::DISCONNECT_LATER)
    {
        if (peer->incoming_bandwidth != 0)
            peer->host->decrease_bandwidth_limited_peers();

        peer->host->decrease_connected_peers();
    }
}

void
UdpPeer::udp_peer_disconnect()
{
    // ...
}

void
UdpPeer::udp_peer_reset_queues()
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
UdpPeer::udp_peer_reset()
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
UdpPeer::udp_peer_receive(uint8_t &channel_id)
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
UdpPeer::udp_peer_ping()
{
    if (peer->state != UdpPeerState::CONNECTED)
        return;

    std::shared_ptr<UdpProtocolType> cmd = std::make_shared<UdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_PING | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    udp_peer_queue_outgoing_command(peer, cmd, nullptr, 0, 0);
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

void
UdpPeer::change_state(const UdpPeerState state)
{
    if (state == UdpPeerState::CONNECTED || state == UdpPeerState::DISCONNECT_LATER)
        udp_peer_on_connect();
    else
        udp_peer_on_disconnect();

    _state = state;
}

bool UdpPeer::is_disconnected()
{
    return state == UdpPeerState::DISCONNECTED ? true : false;
}

Error
UdpPeer::setup(const UdpAddress &address, SysCh channel_count, uint32_t data, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    _channels = std::move(std::vector<std::shared_ptr<UdpChannel>>(static_cast<int>(channel_count)));

    if (_channels.empty())
        return Error::CANT_CREATE;

    _state = UdpPeerState::CONNECTING;
    _address = address;
    _connect_id = hash32();

    if (_outgoing_bandwidth == 0)
        _window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
        _window_size = (_outgoing_bandwidth / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (_window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
        _window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (_window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
        _window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;

    std::shared_ptr<UdpProtocolType> cmd = std::make_shared<UdpProtocolType>();

    cmd->header.command = PROTOCOL_COMMAND_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    cmd->connect.outgoing_peer_id = htons(_incoming_peer_id);
    cmd->connect.incoming_session_id = _incoming_session_id;
    cmd->connect.outgoing_session_id = _outgoing_session_id;
    cmd->connect.mtu = htonl(_mtu);
    cmd->connect.window_size = htonl(_window_size);
    cmd->connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
    cmd->connect.incoming_bandwidth = htonl(_incoming_bandwidth);
    cmd->connect.outgoing_bandwidth = htonl(_outgoing_bandwidth);
    cmd->connect.packet_throttle_interval = htonl(_packet_throttle_interval);
    cmd->connect.packet_throttle_acceleration = htonl(_packet_throttle_acceleration);
    cmd->connect.packet_throttle_deceleration = htonl(_packet_throttle_deceleration);
    cmd->connect.data = data;

    queue_outgoing_command(cmd, nullptr, 0, 0);

    return Error::OK;
}

bool
UdpPeer::needs_dispatch()
{
    return _needs_dispatch;
}

bool
UdpPeer::needs_dispatch(bool val)
{
    _needs_dispatch = val;
}

bool
UdpPeer::acknowledgement_exists()
{
    return !_acknowledgements.empty();
}

std::shared_ptr<UdpAcknowledgement>
UdpPeer::pop_acknowledgement()
{
    auto acknowledgement = _acknowledgements.front();

    _acknowledgements.pop_front();

    return acknowledgement;
}

uint32_t
UdpPeer::mtu()
{
    return _mtu;
}
